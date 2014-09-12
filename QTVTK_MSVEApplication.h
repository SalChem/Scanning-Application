#ifndef QTVTK_MSVEApplication_H
#define QTVTK_MSVEApplication_H

#include <vtkSmartPointer.h>

#include <QMainWindow>

class vtkBorderWidget;
class vtkHandleWidget;
class MyClass;

#include "ui_QTVTK_MSVEApplication.h"

class QTVTK_MSVEApplication : public QMainWindow, private Ui::QTVTK_MSVEApplication
{
	Q_OBJECT
public:
	double *posRightMesh;
	double *posLeftMesh;
	// Constructor/Destructor
	QTVTK_MSVEApplication(); 
	~QTVTK_MSVEApplication() {};
	
	
	void registration();
	void setting_landmarks();

	
	
	public slots:
		
	void on_beforeRightNippleBtn_clicked();
	void on_beforeNavalBtn_clicked();
	void on_beforeLeftNippleBtn_clicked();	
	void on_beforeSternumNotchBtn_clicked();
	void on_afterSternumNotchBtn_clicked();
	void on_afterRightNippleBtn_clicked();
	void on_afterNavalBtn_clicked();
	void on_afterLeftNippleBtn_clicked();
	
	void on_registration4Marks_clicked();
	void on_registration3Marks_clicked();
	
	virtual void slotExit();
	
private:
	vtkSmartPointer<vtkBorderWidget> BorderWidget;
	vtkSmartPointer<vtkHandleWidget> HandleWidgetRight;	
	vtkSmartPointer<vtkHandleWidget> HandleWidgetLeft;		
	// Designer form
	Ui_QTVTK_MSVEApplication *ui;
	
protected:
	void ModifiedHandler();
};

#endif