#include "khtmlcache.h"
#include "khtml.h"

#undef CACHE_DEBUG

#include <qpixmap.h>
#include <qmovie.h>
#include <qdict.h>

#include <sys/stat.h>
#include <unistd.h>
  
KHTMLCachedImage::KHTMLCachedImage()
{
    p = 0;
    m = 0;
    status = KHTMLCache::Unknown;
    size = 0;
}

KHTMLCachedImage::~KHTMLCachedImage()
{
    if( m ) delete m;
    if( p ) delete p;
}

inline void 
KHTMLCachedImage::append( HTMLObject *o ) 
{ 
    clients.append( o ); 
    if( status != KHTMLCache::Pending )
	notify( o );
}

void 
KHTMLCachedImage::remove( HTMLObject *o ) 
{ 
  clients.remove( o ); 
  if(m && clients.isEmpty() && !m->finished())
    m->pause();
}

inline QPixmap *
KHTMLCachedImage::pixmap() 
{
/*    if(m && !p)
    {
	p = new QPixmap();
	*p = m->framePixmap();
    } 
    else if( m )
	*p = m->framePixmap();*/
    return p;
}

inline void
KHTMLCachedImage::computeStatus()
{
    if( size > MAXCACHEABLE && size > KHTMLCache::size()/MAXPERCENT )
    {
	status = KHTMLCache::Uncacheable;
	size = 0;
    }
    else
	status = KHTMLCache::Cached;
}

inline void 
KHTMLCachedImage::clear()
{
    if( m ) {
	delete m;
	m = 0;
    }
    if( p ) {
	delete p;
	p = 0;
    }
    size = 0;
}

void
KHTMLCachedImage::load( const char * _file )
{
    clear();

    // Workaround for bug in QMovie
    // Load the image in memory to avoid vasting file handles
    FILE *f = fopen( _file, "rb" );
    if( !f )
    {
	warning( "Cache: Could not load %s\n", _file );
	return;
    }
    struct stat buff;
    stat( _file, &buff );
    int s = buff.st_size;
    char *c = new char[ s ];
    fread( c, 1, s, f );
    fclose( f );
    QByteArray arr;
    arr.assign( c, s );

    if ( strncmp( c, "GIF89a", 6 ) == 0 )
    {
	m = new QMovie( arr, 8192 );
	p = new QPixmap();
	*p = m->framePixmap();
	// well, this is the size of the compressed gif...
	// ... but that's better than nothing
	size = s;
    }
    else
    {
	p = new QPixmap();
	p->loadFromData( arr );	    
	if( p && !p->isNull() )
	  size = p->width() * p->height() * p->depth() / 8;
    }

    computeStatus();
    notify();
}    

bool
KHTMLCachedImage::data ( QBuffer & _buffer, bool eof )
{
    // no incremental loading for the moment
    if( !eof ) return false;

    clear();

    char buffer[ 7 ];
    buffer[0] = 0;
    _buffer.open( IO_ReadOnly );
    _buffer.readBlock( buffer, 6 );
    _buffer.close();

    if ( strcmp( buffer, "GIF89a" ) == 0 )
    {
	m = new QMovie( _buffer.buffer() );
	p = new QPixmap();
	*p = m->framePixmap();
	size = _buffer.size();
    }
    else
    {
	p = new QPixmap();
	p->loadFromData( _buffer.buffer() );	    
	// set size of image. 
	if( p && !p->isNull() )
	    size = p->width() * p->height() * p->depth() / 8;
    }

    computeStatus();
    notify();
    // FIXME: only update, when needed
    return true;
}

inline void 
KHTMLCachedImage::notify( HTMLObject *o )
{
    if( o )
    {
	// sanity check...
	if( clients.find( o ) == -1 )
	    clients.append( o );
	if( m )
	{
	    o->setMovie( m, p );
	    if(m->finished()) m->restart();
	    if(m->paused()) m->unpause();
	}
	else if ( p != 0 && !p->isNull() )
	    o->setPixmap( p );
	return;
    }

    // notify all objects in our list...
    if( m )
    {
	for( o = clients.first(); o != 0L; o = clients.next() )
	  o->setMovie( m, p );
	return;
    }

    if ( p == 0 || p->isNull() )
	return;
    for( o = clients.first(); o != 0L; o = clients.next() )
	o->setPixmap( p );
}

// --------------------------------------------------------------------------

QDict<KHTMLCachedImage> *KHTMLCache::cache = 0;
ImageList *KHTMLCache::lru = 0;

int KHTMLCache::maxSize = DEFCACHESIZE;
int KHTMLCache::actSize = 0;


KHTMLCache::KHTMLCache( KHTMLWidget *w ) : HTMLObject()
{
    htmlWidget = w;
    init();
}

KHTMLCache::~KHTMLCache()
{
}

void
KHTMLCache::init()
{
    if(!cache)
    {
	cache = new QDict<KHTMLCachedImage>(401, false);
	cache->setAutoDelete(true);
    }
    if(!lru)
    {
	lru = new ImageList;
	// not to delete the image twice...
	lru->setAutoDelete(false);
    }
}

void
KHTMLCache::requestImage( HTMLObject *obj, const char * _url)
{
    // this brings the _url to a standard form...
    KURL kurl( _url );
    if( kurl.isMalformed() )
    {
      printf("Cache: Malformed url: %s\n", _url );
      return;
    }
    KHTMLCachedImage *im = cache->find(kurl.url());
    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("Cache: new: %s\n", _url);
#endif
	im = new KHTMLCachedImage();
	im->status = Pending;
	im->append(obj);
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
	    
	if ( kurl.isLocalFile() )
	{
	    im->load( kurl.path() );
	    actSize += im->size;
	}
	else 
	    htmlWidget->requestFile( this, kurl.url() );
	return;
    }

#ifdef CACHE_DEBUG
    if( im->status == Pending )
	printf("Cache: loading in progress: %s\n", kurl.url().data());
    else
	printf("Cache: using cached: %s\n", kurl.url().data());
#endif

    im->append( obj );
    lru->touch( kurl.url() );
}

void
KHTMLCache::fileLoaded( const char * _url, const char *_file )
{ 
    KHTMLCachedImage *im = cache->find(_url);

    if(!im) 
    {
#ifdef CACHE_DEBUG
	printf("Cache: ERROR loading: %s not found.\n", _url);
#endif
	return;
    }

    // convert file to pixmap or movie
#ifdef CACHE_DEBUG
    printf("Cache: Loaded %s %s\n", _url, _file );
#endif
    
    im->load( _file );
    actSize += im->size;
}

bool
KHTMLCache::fileLoaded( const char *_url, QBuffer & _buffer, bool eof )
{
    KHTMLCachedImage *im = cache->find(_url);

    if(!im) return false;

    // convert file to pixmap or movie
    return im->data( _buffer, eof );
    if( eof )
	actSize += im->size;
}

void 
KHTMLCache::free( const char * _url, HTMLObject *obj )
{
    if(!_url) return;
    KURL kurl( _url );
    KHTMLCachedImage *im = cache->find( kurl.url() );

    if(!im) return;
    im->remove( obj );

#ifdef CACHE_DEBUG
    printf("Cache: references( %s ) = %d\n", _url, im->count());
#endif

    if(im->count() == 0 && (im->status == Pending ||
			    im->status == Uncacheable ) )
    {
	htmlWidget->cancelRequestFile( _url );
	// remove, since it was not fully loaded
	lru->remove( kurl.url() );
	cache->remove( kurl.url() );
#ifdef CACHE_DEBUG
	printf("Cache: deleted %s\n", kurl.url().data());
#endif
    }

}

void
KHTMLCache::flush()
{
    KHTMLCachedImage *im;
    QString url;

    init();

#ifdef CACHE_DEBUG
    statistics();
    printf("Cache: flush()\n");
#endif
    if( actSize < maxSize ) return;
    
    for( url = lru->first(); !url.isEmpty(); url = lru->next() )
    {
	im = cache->find( url );
	if( im->count() || im->status == Persistent ) 
	    continue; // image is still used or cached permanently

#ifdef CACHE_DEBUG
	printf("Cache: removing %s\n", url.data());
#endif
	actSize -= im->size;
	lru->remove( url );
	cache->remove( url );
	if( actSize < maxSize ) break;
    }

#ifdef CACHE_DEBUG
    statistics();
#endif
}

void 
KHTMLCache::preload( const char * _url, Status s)
{
    // better be careful, since this function is static
    init();
    
    KURL kurl( _url );
    if ( kurl.isMalformed() )
    {
	warning("Malformed URL '%s'\n", _url );
	return;
    }	    
    if ( !kurl.isLocalFile() )
    {
#ifdef CACHE_DEBUG
	printf("Cache: image to cache is not local file %s\n", _url);
#endif
	return;
    }

    KHTMLCachedImage *im = cache->find(kurl.url());
    
    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("Cache: *** new cached image %s\n", kurl.path());
#endif
	im = new KHTMLCachedImage();
	im->load( kurl.path() );
	actSize += im->size;
	if( s != Unknown )  // specific status required
	    im->status = s;
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
    }
}


QPixmap *
KHTMLCache::image( const char * _url )
{
    KHTMLCachedImage *im = cache->find(_url);

    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("CACHE: Image not cached: %s\n", _url);
#endif
	return 0;
    }

    return im->pixmap();
}

void 
KHTMLCache::setSize( int bytes )
{
    maxSize = bytes;
    // may be we need to clear parts of the cache
    flush();
}

void 
KHTMLCache::statistics()
{
  KHTMLCachedImage *im;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    QDictIterator<KHTMLCachedImage> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        im = it.current();
	QPixmap *p = im->pixmap();
	if(im->m != 0)
	{
	    movie++;
	    msize += im->size;
	}
	else if( p != 0 && !p->isNull())
	    size += p->width()*p->height()*p->depth()/8;
    }
    size /= 1024;

    printf("------------------------- image cache statistics -------------------\n");
    printf("Number of items in cache: %d\n", cache->count() );
    printf("Number of items in lru  : %d\n", lru->count() );
    printf("Number of cached images: %d\n", cache->count()-movie);
    printf("Number of cached movies: %d\n", movie);
    printf("calculated allocated space approx. %d kB\n", actSize/1024);
    printf("pixmaps:   allocated space approx. %d kB\n", size);
    printf("movies :   allocated space approx. %d kB\n", msize/1024);
    printf("--------------------------------------------------------------------\n");
}
