/**********************************************************************

	--- Qt Architect generated file ---

	File: StringListDialogData.h
	Last generated: Sun Apr 12 18:03:55 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#ifndef StringListDialogData_included
#define StringListDialogData_included

#include <qdialog.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qframe.h>

class StringListDialogData : public QDialog
{
    Q_OBJECT

public:

    StringListDialogData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~StringListDialogData();

public slots:


protected slots:

    virtual void downPressed();
    virtual void addString();
    virtual void deletePressed();
    virtual void upPressed();

protected:
    QListBox* lbStrings;
    QFrame* frameLine;
    QPushButton* buttonOK;
    QPushButton* buttonCancel;
    QPushButton* buttonUp;
    QPushButton* buttonDown;
    QPushButton* buttonDelete;
    QLineEdit* leLine;

};

#endif // StringListDialogData_included