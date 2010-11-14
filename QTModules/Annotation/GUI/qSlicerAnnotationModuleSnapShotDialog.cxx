#include "qSlicerAnnotationModuleSnapShotDialog.h"

#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include <ctkVTKSliceView.h>
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLThreeDView.h"


#include "vtkImageData.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::qSlicerAnnotationModuleSnapShotDialog()
{

  this->m_Logic = 0;

  this->m_vtkImageData = 0;

  this->m_Id = 0;

  ui.setupUi(this);
  createConnection();

}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::~qSlicerAnnotationModuleSnapShotDialog()
{

  if(this->m_Logic)
    {
    this->m_Logic->Delete();
    this->m_Logic = 0;
    }

  if(this->m_vtkImageData)
    {
    this->m_vtkImageData->Delete();
    this->m_vtkImageData = 0;
    }

  if(this->m_Id)
    {
    //delete this->m_Id;
    this->m_Id = 0;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the Annotation module logic here!");
    return;
    }

  this->m_Logic = logic;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::initialize(const char* nodeId)
{
  if (!this->m_Logic)
    {
    qErrnoWarning("initialize: We need the Annotation module logic here!");
    return;
    }

  this->m_Id = nodeId;

  // get the name..
  vtkStdString name = this->m_Logic->GetSnapShotName(this->m_Id);

  // ..and set it in the GUI
  this->ui.nameEdit->setText(name.c_str());

  // get the description..
  vtkStdString description = this->m_Logic->GetSnapShotDescription(this->m_Id);

  // ..and set it in the GUI
  this->ui.descriptionTextEdit->setText(description.c_str());

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSnapShotScreenshotType(this->m_Id);

  // ..and set it in the GUI
  switch(screenshotType)
    {
    case 0:
      // 3D view
      this->ui.threeDViewRadio->setChecked(true);
      break;
    case 1:
      // red Slice view
      this->ui.redSliceViewRadio->setChecked(true);
      break;
    case 2:
      // yellow Slice view
      this->ui.yellowSliceViewRadio->setChecked(true);
      break;
    case 3:
      // green Slice view
      this->ui.greenSliceViewRadio->setChecked(true);
      break;
    default:
      // as fallback, just treat this case as a 3D view
      this->ui.threeDViewRadio->setChecked(true);
    }

  // get the actual screenshot..
  this->m_vtkImageData = this->m_Logic->GetSnapShotScreenshot(this->m_Id);

  // ..and convert it from vtkImageData to QImage..
  QImage qimage;
  this->m_Logic->VtkImageDataToQImage(this->m_vtkImageData,qimage);

  // ..and then to QPixmap..
  QPixmap screenshot;
  screenshot.convertFromImage(qimage, Qt::AutoColor);

  // ..and set it to the gui..
  ui.screenshotPlaceholder->setPixmap(screenshot.scaled(this->ui.screenshotPlaceholder->width(),this->ui.screenshotPlaceholder->height(),
        Qt::KeepAspectRatio,Qt::SmoothTransformation));

  // now we are able to restore a snapshot, so enable the button
  this->ui.restoreButton->setEnabled(true);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::createConnection()
{

  // connect the OK and CANCEL button to the individual Slots
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  this->connect(ui.threeDViewRadio, SIGNAL(clicked()), this, SLOT(onThreeDViewRadioClicked()));
  this->connect(ui.redSliceViewRadio, SIGNAL(clicked()), this, SLOT(onRedSliceViewRadioClicked()));
  this->connect(ui.yellowSliceViewRadio, SIGNAL(clicked()), this, SLOT(onYellowSliceViewRadioClicked()));
  this->connect(ui.greenSliceViewRadio, SIGNAL(clicked()), this, SLOT(onGreenSliceViewRadioClicked()));

  this->connect(ui.restoreButton, SIGNAL(clicked()), this, SLOT(onRestoreButtonClicked()));

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onDialogRejected()
{

  // emit an event which gets caught by main GUI window
  emit dialogRejected();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onDialogAccepted()
{

  // name
  QString name = this->ui.nameEdit->text();
  QByteArray nameBytes = name.toAscii();

  // description
  QString description = this->ui.descriptionTextEdit->toPlainText();
  QByteArray descriptionBytes = description.toAscii();


  // we need to know of which type the screenshot is
  int screenshotType;

  if (this->ui.threeDViewRadio->isChecked())
    {
    screenshotType = 0;
    }
  else if (this->ui.redSliceViewRadio->isChecked())
    {
    screenshotType = 1;
    }
  else if (this->ui.yellowSliceViewRadio->isChecked())
    {
    screenshotType = 2;
    }
  else if (this->ui.greenSliceViewRadio->isChecked())
    {
    screenshotType = 3;
    }

  if (!this->m_Id)
    {
    // this is a new snapshot
    this->m_Logic->CreateSnapShot(nameBytes.data(),descriptionBytes.data(),screenshotType,this->m_vtkImageData);

    QMessageBox::information(this, "Annotation Snap Shot created",
                               "A new annotation snap shot was created and "
                               "the current scene was attached.");

    }
  else
    {
    // this snapshot already exists
    this->m_Logic->ModifySnapShot(this->m_Id,nameBytes.data(),descriptionBytes.data(),screenshotType,this->m_vtkImageData);

    QMessageBox::information(this, "Annotation Snap Shot updated",
                                   "The annotation snap shot was updated without "
                                   "changing the attached scene.");

    }

  // emit an event which gets caught by main GUI window
  emit dialogAccepted();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onThreeDViewRadioClicked()
{
  this->grabScreenShot("");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onRedSliceViewRadioClicked()
{
  this->grabScreenShot("Red");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onYellowSliceViewRadioClicked()
{
  this->grabScreenShot("Yellow");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onGreenSliceViewRadioClicked()
{
  this->grabScreenShot("Green");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onRestoreButtonClicked()
{
  this->m_Logic->RestoreSnapShot(this->m_Id);

  QMessageBox::information(this, "Annotation Snap Shot restored.",
                                 "The annotation snap shot was restored "
                                 "including the attached scene.");

  emit dialogAccepted();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::reset()
{
  this->ui.threeDViewRadio->setChecked(true);
  this->ui.redSliceViewRadio->setChecked(false);
  this->ui.yellowSliceViewRadio->setChecked(false);
  this->ui.greenSliceViewRadio->setChecked(false);
  this->grabScreenShot("");
  this->ui.descriptionTextEdit->clear();
  this->ui.nameEdit->clear();

  this->ui.restoreButton->setEnabled(false);

  // reset the id
  this->m_Id = 0;

}

//-----------------------------------------------------------------------------
// Grab a screenshot of the 3DView or any sliceView.
// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::grabScreenShot(QString screenshotWindow)
{

  QWidget* widget = 0;

  if(screenshotWindow.length()>0)
    {
    // create a screenShot of a specific sliceView
     widget = static_cast<QWidget*>(qSlicerApplication::application()->layoutManager()->sliceWidget(screenshotWindow)->getSliceView());
    }
  else
    {
    // create a screenShot of the first 3DView
    widget = static_cast<QWidget*>(qSlicerApplication::application()->layoutManager()->threeDView(0));
    }

  QPixmap screenShot = QPixmap::grabWidget(widget);

  ui.screenshotPlaceholder->setPixmap(screenShot.scaled(this->ui.screenshotPlaceholder->width(),this->ui.screenshotPlaceholder->height(),
      Qt::KeepAspectRatio,Qt::SmoothTransformation));


  // convert the screenshot from QPixmap to vtkImageData and store it with this class
  vtkImageData* vtkimage = vtkImageData::New();
  this->m_Logic->QImageToVtkImageData(screenShot.toImage(),vtkimage);
  this->m_vtkImageData = vtkimage;

}


