#ifndef KWVIEV_H
#define KWVIEV_H

#include <qscrbar.h>
#include <qiodev.h>
#include <qpopmenu.h>

#include <kfm.h>
#include <kconfig.h>

#include "kwdialog.h"

class KWriteDoc;
class Highlight;

//search flags
const sfCaseSensitive   = 1;
const sfWholeWords      = 2;
const sfFromCursor      = 4;
const sfBackward        = 8;
const sfSelected        = 16;
const sfPrompt          = 32;
const sfReplace         = 64;
const sfAgain           = 128;
const sfWrapped         = 256;
const sfFinished        = 512;
const srYes             = QDialog::Accepted;
const srNo              = 10;
const srAll             = 11;
const srCancel          = QDialog::Rejected;

//config flags
const cfAutoIndent      = 1;
const cfBackspaceIndent = 2;
const cfWordWrap        = 4;
const cfReplaceTabs     = 8;
const cfRemoveSpaces    = 16;
const cfWrapCursor      = 32;
const cfAutoBrackets    = 64;

const cfPersistent      = 128;
const cfKeepSelection   = 256;
const cfVerticalSelect  = 512;
const cfDelOnInput      = 1024;
const cfXorSelect       = 2048;

const cfOvr             = 4096;
const cfMark            = 8192;

//update flags
const ufDocGeometry     = 1;
//const ufNoScroll        = 1;
const ufUpdateOnScroll  = 2;
//const ufCenter          = 4;
const ufPos             = 4;

void resizeBuffer(void *user, int w, int h);

struct PointStruc {
  int x;
  int y;
};

struct VConfig {
  PointStruc cursor;
  int flags;
  int wrapAt;
};

struct SConfig {
  PointStruc cursor;
  PointStruc startCursor;
  int flags;
};

class KWrite;

class KWriteView : public QWidget {
    Q_OBJECT
    friend KWriteDoc;
    friend KWrite;
  public:
    KWriteView(KWrite *, KWriteDoc *);
    ~KWriteView();

    void cursorLeft(VConfig &);
    void cursorRight(VConfig &);
    void cursorUp(VConfig &);
    void cursorDown(VConfig &);
    void home(VConfig &);
    void end(VConfig &);
    void pageUp(VConfig &);
    void pageDown(VConfig &);
    void top(VConfig &);
    void bottom(VConfig &);

  protected slots:
    void changeXPos(int);
    void changeYPos(int);

  protected:
    void getVConfig(VConfig &);
    void update(VConfig &);
//    void updateCursor(PointStruc &start, PointStruc &end, bool insert);
    void insLine(int line);
    void delLine(int line);
    void updateCursor(PointStruc &newCursor);
    void updateView(int flags, int newXPos = 0, int newYPos = 0);
//  void scroll2(int, int);
    void tagLines(int start, int end);
    void tagAll();

    void paintTextLines(int xPos, int yPos);
    void paintCursor();

    void placeCursor(int x, int y, int flags);

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    QScrollBar *xScroll;
    QScrollBar *yScroll;

    int xPos;
    int yPos;

    int mouseX;
    int mouseY;
    int scrollX;
    int scrollY;
    int scrollTimer;

    PointStruc cursor;
    bool cursorOn;
    int cursorTimer;
    int cXPos;
    int cOldXPos;
    bool exposeCursor;//cursorMoved;

    int startLine;
    int endLine;
    int updateState;
    int updateLines[2];

    QPixmap *drawBuffer;
};

class KWBookmark {
  public:
    KWBookmark();
    int xPos;
    int yPos;
    PointStruc cursor;
    QString Name;
};

class KWBookPopup : public QPopupMenu {
    Q_OBJECT
  public:
    KWBookPopup();
    virtual void show();
  signals:
    void exposed();
};

class KWrite : public QWidget {
    Q_OBJECT
    friend KWriteView;
    friend KWriteDoc;
  public:
    KWrite(KWriteDoc *, QWidget *parent=0);
    ~KWrite();

//status functions
    int currentLine();
    int currentColumn();
    int config();
    void setConfig(int);
//    bool isOverwriteMode();
    void setModified(bool);
    bool isModified();
    bool isLastView();
    KWriteDoc *doc();
    int undoState();
    void copySettings(KWrite *);
  public slots:
    void optDlg();
    void toggleVertical();
    void toggleOverwrite();
  signals:
    void newCurPos(); //line, col
    void newStatus(); //modified, config flags
    void statusMsg(const char *);
    void newCaption();
    void newUndo();
  protected:
    int configFlags;
    int wrapAt;

//url aware file functions
  public:
    enum action{GET, PUT}; //tells us what kind of job kwrite is waiting for
    void loadFile(QIODevice &, bool insert = false);
    void writeFile(QIODevice &);
    bool loadFile(const char *name, bool insert = false);
    bool writeFile(const char *name);
    void loadURL(const char *url, bool insert = false);
    void writeURL(const char *url);
  protected slots:
    void kfmFinished();
    void kfmError(int, const char *);
  public:
    const char *fileName();
    bool canDiscard();
  public slots:
    void newDoc();
    void open();
    void insertFile();
    void save();
    void saveAs();
  protected:
    KFM *kfm;
    QString kfmURL;
    QString kfmFile;
    action kfmAction;
    bool kfmInsert;

//edit functions
  public:
    void clear();
  public slots:
    void cut();
    void copy();
    void paste();
    void selectAll();
    void deselectAll();
    void invertSelection();
    void undo();
    void redo();

//search functions
  public slots:
    void search();
    void replace();
    void searchAgain();
    void gotoLine();
  protected:
    void initSearch(SConfig &, int flags);
    void continueSearch(SConfig &);
    void searchAgain(SConfig &);
    void replaceAgain();
    void doReplaceAction(int result, bool found = false);
    void exposeFound(PointStruc &cursor, int slen, int flags, bool replace);
    void deleteReplacePrompt();
    bool askReplaceEnd();
  protected slots:
    void replaceSlot();
  protected:
    QString searchFor;
    QString replaceWith;
    int searchFlags;
    int replaces;
    SConfig s;
    QDialog *replacePrompt;
//right mouse button popup menu
  public:
    void installRBPopup(QPopupMenu *);
  protected:
    QPopupMenu *popup;

//bookmarks
  public:
    void installBMPopup(KWBookPopup *);
    void setBookmark(int n);
  public slots:
    void setBookmark();
    void addBookmark();
    void clearBookmarks();
    void gotoBookmark(int n);
  protected slots:
    void updateBMPopup();
  protected:
    QList<KWBookmark> bookmarks;
//    KWBookPopup *bookPopup;

//config file / session management functions
  public:
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);
    void setHighlight(Highlight *);

//internal
  protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    KWriteView *kWriteView;
    KWriteDoc *kWriteDoc;
};


#endif //KWVIEV_H

