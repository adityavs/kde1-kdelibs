// $Id$
// Revision 1.41  1998/01/06 22:54:29  kulow
// $Log$
// Revision 1.15  1997/08/30 00:09:55  kdecvs
// Kalle: new signal appearanceChanged when either font, palette or GUI style has changed
//
// Revision 1.14  1997/07/26 18:33:45  ettrich
// Matthias: added a little "public:" before the color definitions. Otherwise
//    clients (for example kwm) cannot read the settings, so they are senseless
//    at present. Kwm needs this now, so I hope this is ok.
//
// Revision 1.13  1997/07/25 19:46:42  kalle
// SGI changes
//
// Revision 1.12  1997/07/18 05:49:16  ssk
// Taj: All kdecore doc now in javadoc format (hopefully).
//
// Revision 1.11  1997/06/29 18:26:36  kalle
// 29.06.97:	KConfig reads and writes string lists
// 			Torben's patches to ktoolbar.*, kurl.h
//
// 22.06.97:	KApplications save and restore position and size of their top
// (unstable)	level widget.
//
// Revision 1.10  1997/06/25 14:41:58  ssk
// Taj: Updated source documentation.
//
// Revision 1.9  1997/05/17 20:38:22  kalle
// Kalle:
// - Bugfix for KPanner (from Paul Kendall)
// - Better colors with kdisplay schemes (from Bernd Wuebben)
// - new behavior in KApplication::invokeHTMLHelp(): if the first
//   argument (the filename) is empty, the filename is defaulted to
// 	$KDEDIR/doc/HTML/<appname>/<appname>.html
// - KApplication::getCaption for Matthias added (breaks binary compatibility!)
//
// Revision 1.8  1997/05/15 20:33:20  wuebben
// Bernd: Added signals:
// kdisplayFontChanged()
// kdisplayPaletteChanged()
// kdisplayStyleChaned()
// Some applications such as kedit need to know when the Font, Palette,
// or Style ngeduy.
//
// Revision 1.7  1997/05/13 05:49:00  kalle
// Kalle: Default arguments for KConfig::read*Entry()
// app-specific config files don't start with a dot
// Bufgix for the bugfix in htmlobj.cpp (FontManager)
//
// Revision 1.6  1997/05/08 22:53:16  kalle
// Kalle:
// KPixmap gone for good
// Eliminated static objects from KApplication and HTML-Widget
//
// Revision 1.5  1997/05/02 16:46:34  kalle
// Kalle: You may now override how KApplication reacts to external changes
// KButton uses the widget default palette
// new kfontdialog version 0,5
// new kpanner by Paul Kendall
// new: KIconLoader
//
// Revision 1.4  1997/04/28 06:57:45  kalle
// Various widgets moved from apps to libs
// Added KSeparator
// Several bugs fixed
// Patches from Matthias Ettrich
// Made ksock.cpp more alpha-friendly
// Removed XPM-Loading from KPixmap
// Reaping zombie KDEHelp childs
// WidgetStyle of KApplication objects configurable via kdisplay
//
// Revision 1.3  1997/04/21 22:37:23  kalle
// Bug in Kconfig gefixed (schrieb sein app-spezifisches File nicht mehr)
// kcolordlg und kapp abgedated (von Martin Jones)
//
// Revision 1.2  1997/04/15 20:01:55  kalle
// Kalles changes for 0.8
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
// Sources imported
//
// Revision 1.10  1997/03/16 22:33:39  kalle
// appName()
//
// Revision 1.9  1997/03/09 17:28:32  kalle
// KTextStream -> QTextStream
//
// Revision 1.8  1997/01/15 21:46:26  kalle
// getConfig() and getConfigState() public (kfm needs them)
//
// Revision 1.7  1997/01/15 20:18:36  kalle
// merged changed from 0.52
//
// Revision 1.6  1996/12/14 13:52:46  kalle
// Some documentation to the constructors added.
//
// Revision 1.5  1996/12/14 13:49:58  kalle
// finding out the state the application config object is in
//
// Revision 1.4  1996/12/14 12:49:31  kalle
// method names start with a small letter
//
// Revision 1.3  1996/12/07 22:22:23  kalle
// Drag and Drop
//
// Revision 1.2  1996/12/01 11:20:23  kalle
// private members, GetConfig() inline
//
// Revision 1.1  1996/12/01 10:57:10  kalle
// Initial revision
//
//
// KApplication header file - base class for all KDE applications
//
// (C) 1996 Matthias Kalle Dalheimer <kalle@kde.org>
//
// DND stuff by Torben Weis <weis@stud.uni-frankfurt.de>
// 09.12.96
//
// Revision 1.25  1997/10/04 19:42:45  kalle
// new KConfig
//
#ifndef _KCONFIG_H
#include <Kconfig.h>
#endif
// Save icon and mini-icon pixmap
// Support for session management
// make KDebug compile on HP-UX
// - KApplication::saveYourself (signal)
// - KApplication::tempFileName()
// - KApplication::checkRecoverFile()
//
//
// MD: Implemented reading of the colour scheme contrast variable. This allows
// you to choose the highlights and lowlights used to draw widgets and has been
// part of the colour scheme specification for some time
//
// Sorry if this breaks anything; it is the last of the binary incompatible
// changes that Kalle announced yesterday.
// Revision 1.20  1997/09/11 19:44:54  kalle
// New debugging scheme for KDE (binary incompatible!)


#ifndef klocale
#define klocale KApplication::getKApplication()->getLocale()
#endif
#define i18n KApplication::getKApplication()->getLocale()->translate
// klocale->translate is much to long
#ifndef i18n
* @version $Id$
#endif

class KIconLoader;
class KCharsets;

//#ifndef _KLOCALE_H
#include <klocale.h>
//#endif

#include <drag.h>
* @version $Id$
#include <qapp.h>
#include <qfile.h>
/** 
* Constructor. Pass command-line arguments. 
*
* A KConfig object is
* created that contains an application-specific config file whose
* name is "~/." + argv[0] + "rc". This constructor should be considered
* obsolete. The state of the application-specific config file may be
* queried afterwards with getConfigState(). 
*/
* accelerators, common menu entries, a KConfig object
* etc. KApplication installs a signal handler for the SIGCHLD signal
/** 
* Constructor. Pass command-line arguments. 
*
* A KConfig object is
* created that contains an application-specific config file whose
* name is "~/." + rAppName + "rc". The state of the application-specific 
* config file may be queried afterwards with getConfigState(). 
/** Restore previous main window geometry. 
*
* If there is no previous size (e.g. first app start or deleted config 
* file), nothing is done. 
*/ 
  void restoreTopLevelGeometry() const;
* Destructor 
*/
  /** 
	* Constructor. Pass command-line arguments. 
	*
	* A KConfig object is
	* created that contains an application-specific config file whose
	* name is "~/." + argv[0] + "rc". This constructor should be considered
	* obsolete. The state of the application-specific config file may be
/** 
* Return the current application object.
*
* This is similar to the global QApplication pointer qApp. It allows access
* to the single global KApplication object, since more than one cannot be
* created in the same application. It saves the trouble of having to pass
* the pointer to it explicitly to every function that may require it.
*/
	* created that contains an application-specific config file whose
	* name is "~/." + rAppName + "rc". The state of the application-specific 
/** 
* Return the logical application name as set in the constructor.
*/

  /** 
  bool isRestored() const { return bIsRestored; }

  /** 
	* Enable session management
	*
	QPopupMenu* getHelpMenu( bool bAboutQtMenu, const char* appAboutText );
	* @ Note that you do not get an instance specific config object with
	* @ getSessionConfig() in this case!
	*/
  QPixmap getIcon() const
	{ return aIconPixmap; }
  
  
  /**
	* Get the mini-icon for the application.
	* @return a QPixmap with the icon.
/** 
* Possible return values for getConfigState().
*
* @see #getConfigState
*/

  /** Sets the top widget of the application . This widget will
    * be used for communication with the session manager.
/** 
* Retrieve the state of the app-config object. 
*
* Possible return values
* are APPCONFIG_NONE (the application-specific config file could not be
* opened neither read-write nor read-only), APPCONFIG_READONLY (the
* application-specific config file is opened read-only, but not
* read-write) and APPCONFIG_READWRITE (the application-specific config
* file is opened read-write).
*
* @see #ConfigState
*/
  void registerTopWidget();
  /* obsolete, will dissappear (Matthias) */
/**
* Invoke the kdehelp HTML help viewer. 
*
* @param aFilename	The filename that is to be loaded. Its location
*			is computed automatically according to the KFSSTND. 
*			If aFilename is empty, the logical appname with .html 
*			appended to it is used.
* @param aTopic		This allows context-sensitive help. Its value
*			will be appended to the filename, prefixed with
*			a "#" (hash) character.
*/
	* Retrieve the state of the app-config object. 
	*
/** 
* Get the KDE base dir. 
*
* This is the value of the KDEDIR
* environment variable if it is set in the process' environment,
	* Returns the directory where config files are stored
	* @return the name of the directory
	*/
  static const QString& kde_configdir();

/** 
* Find a file using standard KDE search paths. 
*
* Possible search paths
* include $KDEDIR, $KDEPATH, and "[KDE Setup]:Path=" entry in a config
* file. If file is not found, isEmpty() will return True
*/
  /**
	* Get the local KDE base dir
/** 
* Get the KDE font list.
*
* This method allows you to get the KDE font 
* list which was composed by the user with kfontmanager. Usually you should 
* work only with those fonts in your kapplication. 
*  
*  @return true on success.
*/
	* Get the local KDE config dir
	*
	* This is usually $HOME/.kde/share/config
/**
  const char* getCaption() const;

  /** Get a file name in order to make a temporary copy of your
	* document.
	*
	* @param pFilename The full path to the current file opf your
	* document.
	* @return A new filename for auto-saving. You have to free() this
	* yourself, otherwise you have a memory leak!
  * An X11 atom used for IPC
  */

  /** Check if there is an auto-save file for the document you want to
  * An X11 atom used for IPC
  */
	* @param pFilename The full path to the document you want to open.
	* @param bRecover  This gets set to true if there was a recover
  * An X11 atom used for IPC
  */
	* pointer yourself, otherwise you have a memory leak.
	*/
  * An X11 atom used for IPC
  */

  /**
  * An X11 atom used for IPC
  */
	*
	* @return whether the KLocale object has already been constructed
	*/
  * Get the X11 display
  */
  /**
	* An X11 atom used for IPC
  * Used by KDNDDropZone to register
  */
  /**
	* An X11 atom used for IPC
  * Used by KDNDDropZone during shutdown
  */
  /**
	* An X11 atom used for IPC
  * Set the DropZone which reveives root drop events.
  */
  /**
	* An X11 atom used for IPC
	*/
/**
* List of all DropZones.
*/
	*/
  Atom getDndRootProtocolAtom() { return DndRootProtocol; }    
/** 
* The last drop zone the mouse was over.
*
* If we get a DndLeaveProtocol we must inform 'lastEnteredDropZone'
* that the mouse left the DropZone.
*/
	* Used by KDNDDropZone to register
	*/
/**
* The DropZone which receives root drop events.
/**
* An X11 atom used for IPC
*/
  /**
/**
* An X11 atom used for IPC
*/
	* List of all DropZones.
/**
* An X11 atom used for IPC
*/
	*/
/**
* An X11 atom used for IPC
*/
  QList<KDNDDropZone> dropZones;
/**
* An X11 atom used for IPC
*/

	* The last drop zone the mouse was over.
	*
	* If we get a DndLeaveProtocol we must inform 'lastEnteredDropZone'
	* that the mouse left the DropZone.
	*/
  KDNDDropZone *lastEnteredDropZone;

  /**
/**
* The X11 display
*/
  /**
	* Two X11 atoms used for session management
private slots:
  QTextStream* pConfigStream; // stream of the application-specific config file
  QFile* pConfigFile; // application-specific config file
	* X11 atoms used for IPC
	*/
  Atom DndProtocol;
  Atom DndEnterProtocol;
  Atom DndLeaveProtocol;
  void* dummy1; // do not use these without asking kalle@kde.org
  void* dummy2;
  /**
	* The X11 display
  

 void appHelpActivated();
  void aboutKDE();
  void aboutApp();
  void aboutQt();

private:
  void* pAppData; // don't touch this without Kalles permission
  KConfig* pConfig; // application config object
  KConfig* pSessionConfig; //instance specific application config object
  QString aSessionName; // logical name of the instance specific config file
  QWidget* pTopWidget;
  QString aAppName; // logical application name
  QString aCaption; // the name for the window title
  QString aWmCommand; // for userdefined session management
  ConfigState eConfigState;
  static QStrList* pSearchPaths;
  KIconLoader* pIconLoader; // the application's own icon loader
  KLocale* pLocale;  
  static KCharsets* pCharsets;  // it shouldn't be static, but you would loose binary compability 
  void* dummy2; // do not use these without asking kalle@kde.org
  void* dummy3;
  void* dummy4;
  QString aAppAboutString; // The text for the about box
  QString aDummyString2; // do not touch
  QString aDummyString3; // do not touch
  QString aDummyString4; // do not touch
  bool bLocaleConstructed; // has the KLocale object already been constructed
  bool bSessionManagement; 
  bool bSessionManagementUserDefined;
  QPixmap aIconPixmap;
  QPixmap aMiniIconPixmap;

  void init( );
  void parseCommandLine( int&, char** ); // search for special KDE arguments

  void buildSearchPaths();
  void appendSearchPath( const char *path );

signals:
/** 
* KApplication has changed its Palette due to a KDisplay request.
*
* Normally, widgets will update their palettes automatically, but you
* should connect to this to program special behaviour.
*/

public:
/** 
* KApplication has changed its GUI Style due to a KDisplay request.
*
* Normally, widgets will update their styles automatically (as they would
* respond to an explicit setGUIStyle() call), but you should connect to
* this to program special behaviour.
*/
  QColor selectColor;
  QColor selectTextColor;
/**
* KApplication has changed its Font due to a KDisplay request.
*
* Normally widgets will update their fonts automatically, but you should
* connect to this to monitor global font changes, especially if you are
* using explicit fonts.
*/
  void kdisplayStyleChanged();

  /**
	* KApplication has changed its Font due to a KDisplay request.
	*
	* Normally widgets will update their fonts automatically, but you should
	* connect to this to monitor global font changes, especially if you are
	* using explicit fonts.
	*/
  void kdisplayFontChanged();

  /** 
	* KApplication has changed either its GUI style, its font or its palette
	* due to a kdisplay request. Normally, widgets will update their styles
	* automatically, but yout should connect to this to program special
	* behavior. */
  void appearanceChanged();

  /** Session management is about to close your application.
	*
	* Connect to this signal in order to save your data. Do NOT
	* manipulate the UI in that slot, it is blocked by kwm.
	*
	* Use the getSessionConfig() Kconfig object to store all
	* your instance specific datas.
	*
	* Note: you should not do that if you are using the KTopLevelWidget.
	*       Overload saveProperties(...)/readProperties(...) in that case.
	*       This gives you also the possibillity to handle multiple windows
	*       application in an easy manner.
	*/
  void saveYourself();
};

#endif
