/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qapp.h>
#include <qdrawutl.h>
#include <qmsgbox.h>

#include <kkeyconf.h>
#include "kkeydata.moc"
#include <ckey.h>

void initKKeyConfig( KConfig *pconf )
{
	if ( kKeys )
		return;
	(void)new KKeyConfig(pconf);
}

KKeyConfig* KKeyConfig::pKKeyConfig = 0L;

KKeyConfig::KKeyConfig( KConfig *pconf )
    : aKeyDict(37), aWidgetDict(11)
{
	pKKeyConfig = this;
	if (!pconf)
		fatal("KKeyConfig : Null KConfig object");
	pConfig = pconf;
	aAvailableId = 1;
	aKeyDict.setAutoDelete(TRUE);
	aWidgetDict.setAutoDelete(TRUE);
}

KKeyConfig::~KKeyConfig()
{
	sync();

	/* remove all widgetEntries */
	aWidgetDict.clear();
}

uint KKeyConfig::readCurrentKey( const QString& functionName )
{
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

uint KKeyConfig::readDefaultKey( const QString& functionName )
{
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}


bool KKeyConfig::addKey( const QString& functionName, uint keyCode,
					   bool configurable )
{
	/* search for an existing "functionName" entry */
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( pEntry ) /* already exists : remove it */
		removeKey(functionName);

	pEntry = new KKeyEntry;
	aKeyDict.insert( functionName, pEntry );
	
	pEntry->aDefaultKeyCode = keyCode;
	pEntry->aCurrentKeyCode = keyCode;
	pEntry->bConfigurable = configurable;
	pEntry->aAccelId = 0;
	pEntry->pConnectDict = NULL;

	if ( !configurable )
		return TRUE;

	/* search an entry in the KConfig object */
	pConfig->setGroup("keys");
	if ( !pConfig->hasKey(functionName) )
		return TRUE;
	
	/* read and recognize the key */
	int aKeyCode = stringToKey( pConfig->readEntry(functionName) );
	if ( aKeyCode ) /* if recognized */
		pEntry->aCurrentKeyCode = aKeyCode;
	
	return TRUE;
}

bool KKeyConfig::addKey( const QString& functionName, 
					   const QString& keyCode, bool configurable )
{
	uint iKeyCode = stringToKey(keyCode);
	if ( iKeyCode==0 ) {
		QString str;
		str.sprintf("addKey : unrecognized key string %s", 
					(const char *)keyCode);
		warning(str);
		return FALSE;
	}
	
	return addKey(functionName, iKeyCode, configurable);
}
	
void KKeyConfig::removeKey( const QString& functionName )
{
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	
    if ( !pEntry ) 
		return;
	
	/* disconnect eventuals connections */
	if ( pEntry->aAccelId ) {
		QDictIterator<KKeyConnectEntry> aConnectIt(*pEntry->pConnectDict);
		aConnectIt.toFirst();
		while ( aConnectIt.current() ) {
			disconnectFunction( aConnectIt.currentKey(), functionName );
			++aConnectIt;
		}
	}
	
	aKeyDict.remove( functionName );
}

void KKeyConfig::connectFunction( const QString& widgetName,
							    const QString& functionName,
							    const QObject* receiver, const char* member,
							    bool activate )
{
	QString str;
	
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		str.sprintf( "connectFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	if ( !pEntry ) {
		str.sprintf( "connectFunction : \"%s\" function does not exist",
					 (const char *)functionName );
		warning(str);
		return;
	}
	
	KKeyConnectEntry *pCEntry;
	
	/* if the widget is already connected */
	if ( pEntry->pConnectDict ) {
		pCEntry = (*pEntry->pConnectDict)[ widgetName ];
		if ( pCEntry )
			internalDisconnectFunction( widgetName, pWEntry, pEntry, pCEntry );
	} else {
		pEntry->pConnectDict = new QDict<KKeyConnectEntry>(13);
		pEntry->pConnectDict->setAutoDelete(TRUE);
		pEntry->aAccelId = aAvailableId;
		aAvailableId++;
	}
		
	pCEntry = new KKeyConnectEntry;
	pCEntry->pReceiver = (QObject *)receiver;
	pCEntry->sMember = member;
	pEntry->pConnectDict->insert( widgetName, pCEntry );
	
	pWEntry->createItem( pEntry->aAccelId, pEntry->aCurrentKeyCode,
						 (QObject *)receiver, member );
	
	if ( !activate )
		toggleFunction( widgetName, functionName, FALSE );
}

void KKeyConfig::toggleFunction( const QString& widgetName,
							   const QString& functionName, bool activate )
{	
	QString str;
	
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		str.sprintf( "(dis)activateFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	if ( !pEntry ) {
		str.sprintf( "(dis)activateFunction : \"%s\" function does not exist",
					 (const char *)functionName );
		warning(str);
		return;
	}

	pWEntry->setItemEnabled( pEntry->aAccelId, activate );
}

void  KKeyConfig::disconnectFunction( const QString& widgetName,
								    const QString& functionName )
{
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		QString str;
		str.sprintf( "disconnectFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
    if ( !pEntry ) 
		return;
	/* search for a connection */
	KKeyConnectEntry *pCEntry = (*pEntry->pConnectDict)[ widgetName ];
	if ( !pCEntry ) 
		return;

	internalDisconnectFunction( widgetName, pWEntry, pEntry, pCEntry );
	
	/* if this was the unique connection of the functionName : delete the
	   pConnectDict */
	if ( pEntry->pConnectDict->count()==0 )
		delete pEntry->pConnectDict;
}

void KKeyConfig::internalDisconnectFunction( const QString& widgetName, 
	   KKeyWidgetEntry *pWEntry, KKeyEntry *pEntry, KKeyConnectEntry *pCEntry )
{
	pWEntry->deleteItem( pEntry->aAccelId, pCEntry->pReceiver,
						 pCEntry->sMember );
	pEntry->pConnectDict->remove( widgetName );
}

void KKeyConfig::registerWidget( const QString& widgetName,
							   QWidget *currentWidget )
{
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( pWEntry ) {
		QString str;
		str.sprintf( "initKeyWidget : \"%s\" widget already initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}

	pWEntry = new KKeyWidgetEntry( currentWidget, widgetName );
	aWidgetDict.insert( widgetName, pWEntry );
}

void KKeyConfig::sync()
{
	/* write the current values in aKeyDict to KConfig object */
	pConfig->setGroup("keys");

	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
		if ( aKeyIt.current()->bConfigurable )
			pConfig->writeEntry( aKeyIt.currentKey(),
				keyToString(aKeyIt.current()->aCurrentKeyCode) );
		++aKeyIt;
	}
}

bool KKeyConfig::configureKeys( QWidget *parent )
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	KKeyConfigure kDialog( &aKeyIt, parent );
	
	kDialog.resize(450,320);
	
	if ( kDialog.exec() ) {
		bool modified = FALSE;
		bool activated;
		/* copy all modified configKeyCodes to
		   currentKeyCodes & redo the connections */
		aKeyIt.toFirst();
		#define pE aKeyIt.current()
		while ( pE ) {
			if ( pE->aCurrentKeyCode != pE->aConfigKeyCode ) {
				modified = TRUE;
				pE->aCurrentKeyCode = pE->aConfigKeyCode;
				if ( pE->pConnectDict ) {
					QDictIterator<KKeyConnectEntry> aCIt( *pE->pConnectDict );
					aCIt.toFirst();
					while ( aCIt.current() ) {
						KKeyWidgetEntry *pWE = 
							kKeys->aWidgetDict[ aCIt.currentKey() ];
						activated = pWE->isItemEnabled( pE->aAccelId );
						pWE->deleteItem( pE->aAccelId, 
									aCIt.current()->pReceiver,
									(const char *)aCIt.current()->sMember );
						pWE->createItem( pE->aAccelId, pE->aCurrentKeyCode,
									aCIt.current()->pReceiver,
									(const char *)aCIt.current()->sMember );
						pWE->setItemEnabled( pE->aAccelId, activated );
						++aCIt;
					}
				}
			}
			++aKeyIt;
		}
		
		/* sync these new keys */
		if ( modified ) {
			sync();
			return TRUE;
		}
	}
	
	/* do nothing : no change */
	return FALSE; 
}

void KKeyConfig::disconnectAllFunctions( const QString& widgetName )
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
		disconnectFunction( widgetName, aKeyIt.currentKey() );
		++aKeyIt;
	}
}

void KKeyConfig::destroyWidgetEntry( const QString& widgetName )
{
	aWidgetDict.remove( widgetName );
}


void KKeyConfig::internalDisconnectAll( const QString& widgetName)
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict); 
	aKeyIt.toFirst();
    while ( aKeyIt.current() ) {
		if ( aKeyIt.current()->pConnectDict )
			if ( (*aKeyIt.current()->pConnectDict)[ widgetName ] )
				aKeyIt.current()->pConnectDict->remove( widgetName );
        ++aKeyIt;
	 }
}


/*****************************************************************************/
const QString keyToString( uint keyCode )
{
	QString res;
	
	if ( keyCode & SHIFT )
		res = "SHIFT+";
	if ( keyCode & CTRL )
		res += "CTRL+";
	if ( keyCode & ALT )
		res += "ALT+";
	
	uint kCode = keyCode & ~(SHIFT | CTRL | ALT);

	for (int i=0; i<NB_KEYS; i++) {
		if ( kCode == (uint)KKeys[i].code ) {
			res += KKeys[i].name;
			return res;
		}
	}
	
	return QString((char *)NULL);
}

uint stringToKey(const QString& key )
{
	char *toks[4], *next_tok;
	uint keyCode = 0;
	int j, nb_toks = 0;
	char sKey[200];
	
	strncpy(sKey, (const char *)key, 200);
	next_tok = strtok(sKey,"+");
	
	if ( next_tok==NULL ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks==5 ) return 0;
		next_tok = strtok(NULL, "+");
	} while ( next_tok!=NULL );
	
	/* we test if there is one and only one key (the others tokens
	   are accelerators) ; we also fill the keycode with infos */
	bool  keyFound = FALSE;
	for (int i=0; i<nb_toks; i++) {
		if ( strcmp(toks[i], "SHIFT")==0 )
			keyCode |= SHIFT;
		else if ( strcmp(toks[i], "CTRL")==0 )
		    keyCode |= CTRL;
	    else if ( strcmp(toks[i], "ALT")==0 )
		    keyCode |= ALT;
	    else {
			/* key already found ? */
			if ( keyFound ) return 0;
			keyFound = TRUE;
			
			/* search key */
			for(j=0; j<NB_KEYS; j++) {
				if ( strcmp(toks[i], KKeys[j].name)==0 ) {
				    keyCode |= KKeys[j].code;
					break;
				}
			}
			
			/* key name ot found ? */
			if ( j==NB_KEYS ) return 0;
		}
	}
	
	return keyCode;
}


/*****************************************************************************/
/* KKeyWidgetEntry */
KKeyWidgetEntry::KKeyWidgetEntry( QWidget *widget, const QString& widgetName )
    : QObject()
{
	pWidget = widget;
	sWidgetName = widgetName;
	pAccel = new QAccel( widget );
	
	connect( widget->topLevelWidget(), SIGNAL(destroyed()),
			 SLOT(widgetDestroyed()) );
}

KKeyWidgetEntry::~KKeyWidgetEntry()
{
	kKeys->internalDisconnectAll( sWidgetName );
//	delete pAccel;
//	disconnect( pWidget, 0, this, 0);
}

void KKeyWidgetEntry::widgetDestroyed()
{
	kKeys->destroyWidgetEntry( sWidgetName );
}

void KKeyWidgetEntry::createItem( int accelId, uint keyCode,
								  QObject *receiver, const char *member )
{
	pAccel->insertItem( keyCode, accelId );
	pAccel->connectItem( accelId, receiver, member );
}

void KKeyWidgetEntry::setItemEnabled( int accelId, bool activate )
{
	pAccel->setItemEnabled( accelId, activate );
}

bool KKeyWidgetEntry::isItemEnabled( int accelId )
{
	return pAccel->isItemEnabled( accelId );
}

void KKeyWidgetEntry::deleteItem( int accelId,
								  QObject *receiver, const char *member )
{
	pAccel->disconnectItem( accelId, receiver, member );
	pAccel->removeItem( accelId ); 
}

/*****************************************************************************/
/* SplitListItem                                                             */
/*                                                                           */
/* Added by Mark Donohoe <donohoe@kde.org>                                   */
/*                                                                           */
/*****************************************************************************/

SplitListItem::SplitListItem( const char *s )
	:  QListBoxItem()
{
	setText( s );
	
	QString str( s );
	int i = str.find( ":" );
	
	actionName = str.left( i );
	actionName.simplifyWhiteSpace();
	
	str.remove( 0, i+1 );
	
	keyName = str.simplifyWhiteSpace();
	
	halfWidth = 0;
}

void SplitListItem::setWidth( int newWidth )
{
	halfWidth = newWidth/2;
}

void SplitListItem::paint( QPainter *p )
{
    QFontMetrics fm = p->fontMetrics();
    int yPos;                       // vertical text position
    yPos = fm.ascent() + fm.leading()/2;
    p->drawText( 5, yPos, actionName );
	p->drawText( 5 + halfWidth, yPos, keyName );
}

int SplitListItem::height(const QListBox *lb ) const
{
    return lb->fontMetrics().lineSpacing() + 1;
}

int SplitListItem::width(const QListBox *lb ) const
{
    return lb->fontMetrics().width( text() ) + 6;
}

/*****************************************************************************/
/* SplitList                                                                 */
/*                                                                           */
/* Added by Mark Donohoe <donohoe@kde.org>                                   */
/*                                                                           */
/*****************************************************************************/

SplitList::SplitList( QWidget *parent , const char *name )
	: QListBox( parent, name )
{
	setFocusPolicy( QWidget::StrongFocus );
	if( style() == MotifStyle )
		setFrameStyle( QFrame::Panel | QFrame::Sunken );
	else
		setFrameStyle( QFrame::WinPanel | QFrame::Sunken );

	selectColor = KApplication::getKApplication()->selectColor;
	selectTextColor = KApplication::getKApplication()->selectTextColor;
}

void SplitList::resizeEvent( QResizeEvent *e )
{
	emit newWidth( width() );
	QListBox::resizeEvent( e );
}

void SplitList::styleChange( GUIStyle )
{
	if( style() == MotifStyle )
		setFrameStyle( QFrame::Panel | QFrame::Sunken );
	else
		setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
}

void SplitList::paletteChange ( const QPalette & oldPalette )
{
	selectColor = KApplication::getKApplication()->selectColor;
	selectTextColor = KApplication::getKApplication()->selectTextColor;
	QListBox::paletteChange( oldPalette );
	
}

/*****************************************************************************/
/* KeyButton                                                                 */
/*                                                                           */
/* Added by Mark Donohoe <donohoe@kde.org>                                   */
/*                                                                           */
/*****************************************************************************/

KeyButton::KeyButton( const char* name, QWidget *parent)
	: QPushButton( parent, name )
{
    setFocusPolicy( QWidget::StrongFocus );
	editing = FALSE;
}

KeyButton::~KeyButton ()
{
}

void KeyButton::setText( QString text )
{
	QPushButton::setText( text );
	setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}

void KeyButton::setEdit( bool edit )
{
	editing = edit;
	repaint();
}


void KeyButton::paint( QPainter *painter )
{
	QPointArray a( 4 );
	a.setPoint( 0, 0, 0) ;
	a.setPoint( 1, width(), 0 );
	a.setPoint( 2, 0, height() );
	a.setPoint( 3, 0, 0 );

	QRegion r1( a );
	painter->setClipRegion( r1 );
	painter->setBrush( backgroundColor().light() );
	painter->drawRoundRect( 0, 0, width(), height(), 20, 20);

	a.setPoint( 0, width(), height() );
	a.setPoint( 1, width(), 0 );
	a.setPoint( 2, 0, height() );
	a.setPoint( 3, width(), height() );

	QRegion r2( a );
	painter->setClipRegion( r2 );
	painter->setBrush( backgroundColor().dark() );
	painter->drawRoundRect( 0, 0, width(), height(), 20, 20 );

	painter->setClipping( FALSE );
	qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8, 
								colorGroup(), TRUE, 1, 0L );
	if ( editing ) {
		painter->setPen( colorGroup().base() );
		painter->setBrush( colorGroup().base() );
	} else {
		painter->setPen( backgroundColor() );
		painter->setBrush( backgroundColor() );
	}
	painter->drawRect( 7, 5, width() - 14, height() - 10 ); 

	drawButtonLabel( painter );
	
	painter->setPen( colorGroup().text() );
	painter->setBrush( NoBrush );
	if( hasFocus() || editing ) {
		painter->drawRect( 8, 6, width() - 16, height() - 12 );
	}	
}

/*****************************************************************************/
/* KKeyConfigure                                                             */
/*                                                                           */
/* Originally by Nicolas Hadacek <hadacek@kde.org>                           */
/*                                                                           */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                           */
/*****************************************************************************/

#define V_SPACING          12
#define H_SPACING          20
#define LIST_WIDTH         100
#define LIST_HEIGHT        100
#define H_DEC              5
#define TEXT_HEIGHT        25
#define CHANGE_AREA_WIDTH  270
#define CHANGE_AREA_HEIGHT 70
#define FRAME_WIDTH        15
#define FRAME_HEIGHT       5
#define ALL_DEFAULT_WIDTH  90
#define BIG_BUTTON_WIDTH   60
#define BIG_BUTTON_HEIGHT  40
#define BIG_BUTTON_DEC     20
#define CHECK_BOX_WIDTH    50
#define CHANGE_BUTTON_WIDTH 130
#define EDIT_BUTTON_WIDTH  70
 
KKeyConfigure::KKeyConfigure( QDictIterator<KKeyEntry> *aKeyIt,
							  QWidget *parent )
    : QDialog( parent, 0, TRUE )
{
	setCaption("Configure key bindings");
	bKeyIntercept = FALSE;
	setFocusPolicy( QWidget::StrongFocus );
	
	// TOP LAYOUT MANAGER
	
	// The following layout is used for the dialog
	// 		LIST LABELS LAYOUT
	//		SPLIT LIST BOX WIDGET
	//		CHOOSE KEY GROUP BOX WIDGET
	//		BUTTONS LAYOUT
	// Items are added to topLayout as they are created.
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 ); 
	
	// CREATE LIST LABELS
	
	QBoxLayout *labels = new QHBoxLayout();
	topLayout->addLayout( labels );
	
	actLabel = new QLabel(this);
	labels->addWidget( actLabel );
	
	actLabel->setFrameStyle( QFrame::NoFrame );
	actLabel->setText("Action");
	actLabel->setFixedHeight( actLabel->sizeHint().height() );
	
	keyLabel = new QLabel(this);
	labels->addWidget(keyLabel);
	
	keyLabel->setFrameStyle( QFrame::NoFrame );
	keyLabel->setText("Key");
	keyLabel->setFixedHeight( keyLabel->sizeHint().height() );
	
	// CREATE SPLIT LIST BOX
	
	// Copy all currentKeyCodes to configKeyCodes
	// and fill up the split list box with the action/key pairs.
	
	wList = new SplitList( this );
	topLayout->addWidget( wList, 15 );
	
	wList->setAutoUpdate(FALSE);
	wList->setFocus();
	
	aIt = aKeyIt;
	aIt->toFirst();
	while ( aIt->current() ) {
		aIt->current()->aConfigKeyCode = aIt->current()->aCurrentKeyCode;
		
		SplitListItem *sli = new SplitListItem(
		 	item( aIt->current()->aConfigKeyCode, aIt->currentKey() )
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
		wList->insertItem( sli );
		
		++ ( *aIt );
	}

	if ( wList->count() == 0 ) wList->setEnabled( FALSE );
	//connect( wList, SIGNAL( selected( int ) ), SLOT( toChange( int ) ) );
	connect( wList, SIGNAL( highlighted( int ) ), SLOT( toChange( int ) ) );
	
	// CREATE CHOOSE KEY GROUP
	
	fCArea = new QGroupBox( this );
	topLayout->addWidget( fCArea, 1 );
	
	fCArea->setTitle( "Choose Key" );
	fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );
	
	// CHOOSE KEY GROUP LAYOUT MANAGER
	
	QGridLayout *grid = new QGridLayout( fCArea, 5, 4, 2 );
	
	grid->setRowStretch(0,20);
	grid->setRowStretch(1,10);
	grid->setRowStretch(2,10);
	grid->setRowStretch(3,10);
	grid->setRowStretch(4,10);

	grid->setColStretch(0,10);
	grid->setColStretch(1,10);
	grid->setColStretch(2,10);
	grid->setColStretch(3,10);
	
	bChange = new KeyButton("key", fCArea);
	bChange->setEnabled( FALSE );
	connect( bChange, SIGNAL( clicked() ), SLOT( changeKey() ) );
	
	cShift = new QCheckBox( fCArea );
	cShift->setText( "SHIFT" );
	cShift->setEnabled( FALSE );
	connect( cShift, SIGNAL( clicked() ), SLOT( shiftClicked() ) );
	
	cCtrl = new QCheckBox( fCArea );
	cCtrl->setText( "CTRL" );
	cCtrl->setEnabled( FALSE );
	connect( cCtrl, SIGNAL( clicked() ), SLOT( ctrlClicked() ) );
	
	cAlt = new QCheckBox( fCArea );
	cAlt->setText( "ALT" );
	cAlt->setEnabled( FALSE );
	connect( cAlt, SIGNAL( clicked() ), SLOT( altClicked() ) );
	
	// Set height of checkboxes to basic key button height.
	// Basic key button height = push button height.
	
	cAlt->setFixedHeight( bChange->sizeHint().height() );
	cShift->setFixedHeight( bChange->sizeHint().height() );
	cCtrl->setFixedHeight( bChange->sizeHint().height() );
	
	fCArea->setMinimumHeight( 4*bChange->sizeHint().height() );
	
	// Add widgets to the geometry manager
	
	grid->addWidget( cShift, 1, 1 );
	grid->addWidget( cCtrl, 2, 1 );
	grid->addWidget( cAlt, 3, 1 );
	grid->addMultiCellWidget( bChange, 2,3, 2,2 );
	
	
	// CREATE THE BUTTONS
	
	QBoxLayout *buttons = new QHBoxLayout();
    topLayout->addLayout( buttons );
	
	bHelp = new QPushButton( this );
	bHelp->setText( "Help" );
	bHelp->setEnabled( FALSE );
	
	bDefault = new QPushButton( this );
	bDefault->setText( "Default" );
	bDefault->setEnabled( FALSE );
	connect( bDefault, SIGNAL( clicked() ), SLOT( defaultKey() ) );
	
	bAllDefault = new QPushButton( this );
	bAllDefault->setText( "All Default ..." );
	bAllDefault->setEnabled( FALSE );
	connect( bAllDefault, SIGNAL( clicked() ), SLOT( allDefault() ) );
	if ( wList->count()==0 ) bAllDefault->setEnabled( FALSE );
	
	bOk = new QPushButton( this );
	bOk->setText( "OK" );
	connect( bOk, SIGNAL( clicked() ), SLOT( accept() ) );
	
	bCancel = new QPushButton( this );
	bCancel->setText( "Cancel" );
	connect( bCancel, SIGNAL( clicked() ), SLOT( reject() ) );
	
	// Find widest button and set all buttons to this width
	
	int widget_width = 0;
	
	if( bHelp->sizeHint().width() > widget_width )
		widget_width =  bHelp->sizeHint().width();
	
	if( bOk->sizeHint().width() > widget_width )
		widget_width =  bOk->sizeHint().width();
		
	if( bCancel->sizeHint().width() > widget_width )
		widget_width =  bCancel->sizeHint().width();
		
	if( bAllDefault->sizeHint().width() > widget_width )
		widget_width =  bAllDefault->sizeHint().width();
		
	if( bDefault->sizeHint().width() > widget_width )
		widget_width =  bDefault->sizeHint().width();
	
	bHelp->setFixedSize( widget_width, bOk->sizeHint().height() );	
	bOk->setFixedSize( widget_width, bOk->sizeHint().height() );
	bCancel->setFixedSize( widget_width, bOk->sizeHint().height() );
	bAllDefault->setFixedSize( widget_width, bOk->sizeHint().height() );
	bDefault->setFixedSize( widget_width, bOk->sizeHint().height() );
	
	// Add buttons to the geometry manager.
	
	buttons->addWidget( bHelp, 0, AlignBottom );
	buttons->addWidget( bDefault, 0, AlignBottom );
	buttons->addWidget( bAllDefault, 0, AlignBottom );
    buttons->addStretch( 10 );
	buttons->addWidget( bOk, 0, AlignBottom );
	buttons->addWidget( bCancel, 0, AlignBottom );

	// eKey, lNotConfig, lInfo are legacy widgets
	// They will be removed.
	
	//eKey = new QLineEdit(fCArea);
	//eKey->resize(0,0);
	//eKey->setMaxLength(MAX_KEY_LENGTH);
	//eKey->hide( );
	//connect(eKey, SIGNAL(returnPressed()), SLOT(editEnd()));
	
	lNotConfig = new QLabel(fCArea);
	lNotConfig->resize(0,0);
	lNotConfig->setFont( QFont("Helvetica", 14, QFont::Bold) );
	lNotConfig->setAlignment( AlignCenter );
	lNotConfig->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	if ( wList->count()==0 )
		lNotConfig->setText("No keys defined");
	else {
		lNotConfig->setText("Not configurable");
		lNotConfig->hide();
	}
	lNotConfig->hide();

	lInfo = new QLabel(fCArea);
	resize(0,0);
	lInfo->setAlignment( AlignCenter );
	lInfo->setEnabled( FALSE );
	lInfo->hide();
	
	wList->setAutoUpdate(TRUE);
	wList->update();
	
	topLayout->activate();
}

KKeyConfigure::~KKeyConfigure()
{
	delete wList;
}

void KKeyConfigure::toChange(int index)
{
	bKeyIntercept = FALSE;
	
	/* get the entry */
	aIt->toFirst();
	(*aIt) += index;
	sEntryKey = aIt->currentKey();
	pEntry = aIt->current();
	
	//eKey->setEnabled( FALSE );
	
	/* is the key configurable ? */
	if ( !pEntry->bConfigurable) {
		lInfo->setEnabled( FALSE );
		cShift->setEnabled( FALSE ); cCtrl->setEnabled( FALSE ); cAlt->setEnabled( FALSE );
		bChange->setEnabled( FALSE );  bDefault->setEnabled( FALSE );
		lNotConfig->setEnabled( TRUE );
		
		uint kCode = pEntry->aConfigKeyCode;
		uint kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		if ( kSCode == Key_Shift ) cShift->setChecked(FALSE);
		else cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) cCtrl->setChecked(FALSE);
		else cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) cAlt->setChecked(FALSE);
		else cAlt->setChecked( kCode & ALT );
		
		QString str = keyToString( kSCode );
		bChange->setText(str);
	} else {
		lNotConfig->setEnabled( FALSE );
		lInfo->setText(""); lInfo->setEnabled( TRUE );
		
		uint kCode = pEntry->aConfigKeyCode;
		uint kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		cShift->setEnabled( TRUE ); cCtrl->setEnabled( TRUE ); cAlt->setEnabled( TRUE );
		if ( kSCode == Key_Shift ) cShift->setChecked(FALSE);
		else cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) cCtrl->setChecked(FALSE);
		else cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) cAlt->setChecked(FALSE);
		else cAlt->setChecked( kCode & ALT );
		
		QString str = keyToString( kSCode );
		bChange->setText(str); //eKey->setText(str);
		bChange->setEnabled( TRUE ); bDefault->setEnabled( TRUE );
		
		if ( isKeyPresent() ) {
			lInfo->setText("Attention : key already used");
		}
	}
}

void KKeyConfigure::fontChange( const QFont & )
{
	actLabel->setFixedHeight( actLabel->sizeHint().height() );
	keyLabel->setFixedHeight( keyLabel->sizeHint().height() );

	cAlt->setFixedHeight( bChange->sizeHint().height() );
	cShift->setFixedHeight( bChange->sizeHint().height() );
	cCtrl->setFixedHeight( bChange->sizeHint().height() );
	
	fCArea->setMinimumHeight( 4*bChange->sizeHint().height() );
	
	int widget_width = 0;
	
	if( bHelp->sizeHint().width() > widget_width )
		widget_width =  bHelp->sizeHint().width();
	
	if( bOk->sizeHint().width() > widget_width )
		widget_width =  bOk->sizeHint().width();
		
	if( bCancel->sizeHint().width() > widget_width )
		widget_width =  bCancel->sizeHint().width();
		
	if( bAllDefault->sizeHint().width() > widget_width )
		widget_width =  bAllDefault->sizeHint().width();
		
	if( bDefault->sizeHint().width() > widget_width )
		widget_width =  bDefault->sizeHint().width();
	
	bHelp->setFixedSize( widget_width, bOk->sizeHint().height() );	
	bOk->setFixedSize( widget_width, bOk->sizeHint().height() );
	bCancel->setFixedSize( widget_width, bOk->sizeHint().height() );
	bAllDefault->setFixedSize( widget_width, bOk->sizeHint().height() );
	bDefault->setFixedSize( widget_width, bOk->sizeHint().height() );
	
	setMinimumWidth( 20+5*(widget_width+10) );
}


void KKeyConfigure::defaultKey()
{
	/* change the configKeyCode */
	pEntry->aConfigKeyCode = pEntry->aDefaultKeyCode;
	
	/* update the list and the change area */
	
	SplitListItem *sli = new SplitListItem(
		 item(pEntry->aConfigKeyCode, sEntryKey)
	);
		
	connect( wList, SIGNAL( newWidth( int ) ),
			 	sli, SLOT( setWidth( int ) ) );
				
	sli->setWidth( wList->width() );

	wList->changeItem( sli, wList->currentItem()  );
	toChange(wList->currentItem());
}

void KKeyConfigure::allDefault()
{
	// Change all configKeyCodes to default values
	
	// NB. There is a bug with this method. If an item in the list
	// is selected then only this item is found in the dictionary
	//
	
	aIt->toFirst();
	wList->clear();
	while ( aIt->current() ) {
		aIt->current()->aConfigKeyCode = aIt->current()->aDefaultKeyCode;
		
		SplitListItem *sli = new SplitListItem(
		 	item(aIt->current()->aConfigKeyCode, aIt->currentKey())
		);
		
		debug("%s", sli->text());
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				 
					
		sli->setWidth( wList->width() );
		
		wList->insertItem( sli );
		
		++(*aIt);
	}
}

#define MAX_FCTN_LENGTH 20

const QString KKeyConfigure::item( uint keyCode, const QString& entryKey )
{
	QString str = entryKey;
	str = str.leftJustify(MAX_FCTN_LENGTH, ' ', TRUE);
	str += " : ";
	str += keyToString(keyCode);
	str = str.leftJustify( MAX_FCTN_LENGTH + 3 + 
						   MAX_KEY_LENGTH+MAX_KEY_MODIFIER_LENGTH, ' ', TRUE );
	return str;
}

void KKeyConfigure::shiftClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Shift ) {
		if ( cShift->isOn() )
			pEntry->aConfigKeyCode |= SHIFT;
		else
			pEntry->aConfigKeyCode &= ~SHIFT;
			
		SplitListItem *sli = new SplitListItem(
		 	item(pEntry->aConfigKeyCode, sEntryKey)
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				 
					
		sli->setWidth( wList->width() );
		
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::ctrlClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Control ) {
		if ( cCtrl->isOn() )
			pEntry->aConfigKeyCode |= CTRL;
		else
			pEntry->aConfigKeyCode &= ~CTRL;
			
		SplitListItem *sli = new SplitListItem(
		 	item(pEntry->aConfigKeyCode, sEntryKey)
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				 
					
		sli->setWidth( wList->width() );
		
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::altClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Alt ) {
		if ( cAlt->isOn() )
			pEntry->aConfigKeyCode |= ALT;
		else
			pEntry->aConfigKeyCode &= ~ALT;
			
		SplitListItem *sli = new SplitListItem(
		 	item(pEntry->aConfigKeyCode, sEntryKey)
		);
		
		connect( wList, SIGNAL( newWidth( int ) ),
				 sli, SLOT( setWidth( int ) ) );
				 
					
		sli->setWidth( wList->width() );
			
		wList->changeItem( sli, wList->currentItem() );
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::changeKey()
{
	bChange->setEdit( TRUE );
	lInfo->setText("Press the wanted key");
	lInfo->setEnabled( TRUE );
	/* give the focus to the widget */
	
	//eKey->setGeometry(bChange->x()+6, bChange->y()+4, bChange->width()-12,
		//bChange->height()-8);
	//eKey->show();
	//eKey->setEnabled(TRUE);
	setFocus();
	bKeyIntercept = TRUE;
}

void KKeyConfigure::keyPressEvent( QKeyEvent *e )
{
	/* the keys are processed if the change button was pressed */
	if ( !bKeyIntercept )
		return;
	
	uint kCode = e->key() & ~(SHIFT | CTRL | ALT);
	/* check the given key :
	   if it is a non existent key (=0) : keep the old value and say
	   what happened. */
	if ( keyToString(kCode).isNull() ) {
		lInfo->setText("Undefined key");
		return;
	}
	
	bKeyIntercept = FALSE;
	//eKey->hide();
	//eKey->setEnabled(FALSE);
	bChange->setEdit(FALSE);
	bChange->setFocus();
	setKey(kCode);
}

void KKeyConfigure::setKey( uint kCode)
{
	// uint kOldCode = pEntry->aConfigKeyCode;
	
	/* add the current modifier to the key */
	if ( kCode!=Key_Shift ) kCode |= (pEntry->aConfigKeyCode & SHIFT);
	if ( kCode!=Key_Control ) kCode |= (pEntry->aConfigKeyCode & CTRL);
	if ( kCode!=Key_Alt ) kCode |= (pEntry->aConfigKeyCode & ALT);
	
	/* set the list and the chande button */
	pEntry->aConfigKeyCode = kCode;
	
	if ( isKeyPresent() ) {
		lInfo->setText("Attention : key already used");
		return;
	}
	
	SplitListItem *sli = new SplitListItem(
	 	item(pEntry->aConfigKeyCode, sEntryKey)
	);
		
	connect( wList, SIGNAL( newWidth( int ) ),
			 sli, SLOT( setWidth( int ) ) );
				 
				
	sli->setWidth( wList->width() );
	
	wList->changeItem( sli, wList->currentItem() );
	toChange(wList->currentItem());
}

void KKeyConfigure::editKey()
{
	bChange->setEnabled( FALSE ); //eKey->setEnabled( TRUE ); 
	lInfo->setText("Return to end edition");
}

void KKeyConfigure::editEnd()
{
	debug("Called editEnd() which relies on eKey widget");
	
	//uint kCode = stringToKey(eKey->text());
	uint kCode = 0;
	if ( kCode==0 || (kCode & (SHIFT | CTRL | ALT)) ) {
		lInfo->setText("Incorrect key");
		return;
	}
	setKey(kCode);
}

bool KKeyConfigure::isKeyPresent()
{
	/* search the aConfigKeyCodes to find if this keyCode is already used
	   elsewhere */
	aIt->toFirst();
	while ( aIt->current() ) {
		if ( aIt->current()!=pEntry
				&& aIt->current()->aConfigKeyCode==pEntry->aConfigKeyCode ) {
			QString str( item(pEntry->aConfigKeyCode, sEntryKey) );
			int i = str.find( ":" );

			QString actionName = str.left( i );
			actionName.simplifyWhiteSpace();

			str.remove( 0, i+1 );

			QString keyName = str.simplifyWhiteSpace();
			
			str.sprintf(
				"The  %s  key combination has already been allocated\nto the  %s  action.\n\nPlease choose a unique key combination.",
				keyName.data(),
				actionName.data() );
				
			QMessageBox::warning( this, "Warning", str.data() );
			
			return TRUE;
		}
		++(*aIt);
	}
	return FALSE;
}   