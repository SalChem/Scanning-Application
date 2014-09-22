#include "QTVTK_MSVEApplication.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPolyDataCollection.h"
#include "vtkCommand.h"
#include <QString> 
#include <vtkVersion.h>
#include <vtkPLYReader.h>
#include <vtkActor.h>
#include <vtkCleanPolyData.h>
#include <vtkCamera.h>
#include <vtkCleanPolyData.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkScalarBarActor.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkDataObjectToTable.h>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkQtTableView.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkPointPicker.h>
#include <vtkHandleWidget.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkBorderWidget.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <QFile>
#include <QMessageBox>
#include <QtDebug>
#include <string>
#include <list>
#include <iostream>
#include <tr1/array>
#include <stdlib.h>

using namespace std;

double centerReferenceLeft[3], centerReferenceRight[3];
double posBeforeSternumNotch[3],posBeforeRightNipple[3],posAfterRightNipple[3],posBeforeLeftNipple[3],posAfterSternumNotch[3],posAfterLeftNipple[3],posBeforeNaval[3],posAfterNaval[3];
double *position;

bool use_stored_landmarks = false;

vtkSmartPointer<vtkPolyData> input1;
vtkSmartPointer<vtkPolyData> input2;

vtkSmartPointer<vtkPoints> sourcePoints =
vtkSmartPointer<vtkPoints>::New();

vtkSmartPointer<vtkPoints> targetPoints =
vtkSmartPointer<vtkPoints>::New();

// Constructor
QTVTK_MSVEApplication::QTVTK_MSVEApplication() 
{
	this->setupUi(this);
	
	//PLY 1	
	vtkSmartPointer<vtkPLYReader> reader1 = vtkSmartPointer<vtkPLYReader>::New();
	reader1->SetFileName("Before_Nahom.ply");
	reader1->Update();
	input1 = reader1->GetOutput();
	
	//PLY 2	
	vtkSmartPointer<vtkPLYReader> reader2 = vtkSmartPointer<vtkPLYReader>::New();
	reader2->SetFileName("After_Nahom.ply");
	reader2->Update();
	input2 = reader2->GetOutput();
	
	input1->GetCenter(centerReferenceLeft);	
	input2->GetCenter(centerReferenceRight);	
	
	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper1->SetInputData(input1);
	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	
	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper2->SetInputData(input2);
	vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
	actor2->SetMapper(mapper2);
	
	// VTK Renderer
	vtkSmartPointer<vtkRenderer> leftRenderer = 
	vtkSmartPointer<vtkRenderer>::New();
	leftRenderer->AddActor(actor1);
	
	vtkSmartPointer<vtkRenderer> rightRenderer = 
	vtkSmartPointer<vtkRenderer>::New();
	
	// Add Actor to renderer
	rightRenderer->AddActor(actor2);
	
	// VTK/Qt wedded
	this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(leftRenderer);
	this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(rightRenderer);
	
	leftRenderer->GetActiveCamera()->SetPosition(0,-1,0);
	leftRenderer->GetActiveCamera()->SetFocalPoint(0,0,0);
	leftRenderer->ResetCamera();
	
	rightRenderer->GetActiveCamera()->SetPosition(0,-1,0);
	rightRenderer->GetActiveCamera()->SetFocalPoint(0,0,0);
	rightRenderer->ResetCamera();
	
	rightRenderer->SetActiveCamera(leftRenderer->GetActiveCamera());	
	this->qvtkWidgetLeft->GetRenderWindow()->AddObserver(vtkCommand::AnyEvent, this, &QTVTK_MSVEApplication::ModifiedHandler);
	
	// Add a Handle widget to the right renderer
	this->HandleWidgetRight = vtkSmartPointer<vtkHandleWidget>::New();
	this->HandleWidgetRight->SetInteractor(this->qvtkWidgetRight->GetInteractor());
	this->HandleWidgetLeft = vtkSmartPointer<vtkHandleWidget>::New();
	this->HandleWidgetLeft->SetInteractor(this->qvtkWidgetLeft->GetInteractor());
	
	vtkPointHandleRepresentation3D *rep = vtkPointHandleRepresentation3D::SafeDownCast(HandleWidgetLeft->GetRepresentation());
	HandleWidgetLeft->EnableAxisConstraintOff();
	
	vtkPointHandleRepresentation3D *rep2 = vtkPointHandleRepresentation3D::SafeDownCast(HandleWidgetRight->GetRepresentation());
	HandleWidgetRight->EnableAxisConstraintOff();
	
	rep2->SetWorldPosition(centerReferenceLeft);
	rep->GetProperty()->SetColor(0.0, 1.0, 1.0);
	rep->GetProperty()->SetLineWidth(1.0);
	rep->GetSelectedProperty()->SetColor(1.0, 1.0, 0.2);
	
	rep2->SetWorldPosition(centerReferenceRight);
	rep2->GetProperty()->SetColor(0.0, 0.0, 1.0);
	rep2->GetProperty()->SetLineWidth(1.0);
	rep2->GetSelectedProperty()->SetColor(1.0, 0.0, 0.0);
	
	this->HandleWidgetLeft->On();	
	
	this->HandleWidgetRight->On();	
	
	vtkSmartPointer<vtkPolygonalSurfacePointPlacer>  pointPlacer = vtkSmartPointer<vtkPolygonalSurfacePointPlacer>::New();
	pointPlacer->AddProp(actor1);
	pointPlacer->GetPolys()->AddItem(input1);
	rep->SetPointPlacer(pointPlacer);
	
	vtkSmartPointer<vtkPolygonalSurfacePointPlacer>  pointPlacer2 = vtkSmartPointer<vtkPolygonalSurfacePointPlacer>::New();
	pointPlacer2->AddProp(actor2);
	pointPlacer2->GetPolys()->AddItem(input2);
	rep2->SetPointPlacer(pointPlacer2);
	
	posLeftMesh = rep->GetWorldPosition();
	posRightMesh = rep2->GetWorldPosition();
	//std::cout << "Point position: " << pos[0] << " " << pos[1] << " " << pos[2];
	
	QString path = QCoreApplication::applicationDirPath();
	path.append("/P.txt");
	QFile file(path);
	
	if(!file.open(QIODevice::ReadOnly)) {
		//Create new file to save landmarks
		
		qDebug() <<"Creating new file to save landmarks..." << path;
	}
	else {
		QMessageBox msgBox;
		msgBox.setText("Landmarks for this image already exist.");
		msgBox.setInformativeText("Use previously selected landmarks?");
		QPushButton *newLandmarksButton = msgBox.addButton(tr("Select New"), QMessageBox::ActionRole);
		msgBox.setStandardButtons(QMessageBox::Yes);
		msgBox.setDefaultButton(QMessageBox::Yes);
		
		int ret = msgBox.exec();
		
		if (msgBox.clickedButton() == newLandmarksButton) {
			qDebug() <<"Unable to open file";
			//Create new file to save landmarks
			//The file is created and landmarks are saved right before registration, so a flag is set here
		} else if (ret == QMessageBox::Yes) {
			//Read Landmarks from file
			//However, reading is done right before registration, so a flag is set here
			qDebug() <<"Opening File...";
			use_stored_landmarks =  true;
		}
		
	}
	//file.close();
	
	
	// Set up action signals and slots
	connect(this->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
}
void QTVTK_MSVEApplication::ModifiedHandler() 
{
	this->qvtkWidgetRight->GetRenderWindow()->Render();
}

void QTVTK_MSVEApplication::on_beforeSternumNotchBtn_clicked()
{
	posBeforeSternumNotch[0] = posLeftMesh[0];
	posBeforeSternumNotch[1] = posLeftMesh[1];
	posBeforeSternumNotch[2] = posLeftMesh[2];
	std::cout << "Position of sternal notch on first mesh: " << posLeftMesh[0] << " " << posLeftMesh[1] << " " << posLeftMesh[2]<< "\n";
}

void QTVTK_MSVEApplication::on_beforeRightNippleBtn_clicked()
{
	posBeforeRightNipple[0] = posLeftMesh[0];
	posBeforeRightNipple[1] = posLeftMesh[1];
	posBeforeRightNipple[2] = posLeftMesh[2];
	std::cout << "Position of right nipple on first mesh: " << posLeftMesh[0] << " " << posLeftMesh[1] << " " << posLeftMesh[2]<< "\n";
}

void QTVTK_MSVEApplication::on_beforeLeftNippleBtn_clicked()
{
	std::cout << "Position of left nipple on first mesh: " << posLeftMesh[0] << " " << posLeftMesh[1] << " " << posLeftMesh[2]<< "\n";
	posBeforeLeftNipple[0] = posLeftMesh[0];
	posBeforeLeftNipple[1] = posLeftMesh[1];
	posBeforeLeftNipple[2] = posLeftMesh[2];
}

void QTVTK_MSVEApplication::on_beforeNavalBtn_clicked()
{
	std::cout << "Position of naval on first mesh: " << posLeftMesh[0] << " " << posLeftMesh[1] << " " << posLeftMesh[2]<< "\n";
	posBeforeNaval[0] = posLeftMesh[0];
	posBeforeNaval[1] = posLeftMesh[1];
	posBeforeNaval[2] = posLeftMesh[2];	
}

void QTVTK_MSVEApplication::on_afterSternumNotchBtn_clicked()
{
	posAfterSternumNotch[0] = posRightMesh[0];
	posAfterSternumNotch[1] = posRightMesh[1];
	posAfterSternumNotch[2] = posRightMesh[2];
	std::cout << "Position of sternal notch on first mesh: " << posRightMesh[0] << " " << posRightMesh[1] << " " << posRightMesh[2]<< "\n";
}

void QTVTK_MSVEApplication::on_afterRightNippleBtn_clicked()
{
	posAfterRightNipple[0] = posRightMesh[0];
	posAfterRightNipple[1] = posRightMesh[1];
	posAfterRightNipple[2] = posRightMesh[2];
	std::cout << "Position of right nipple on second mesh: " << posRightMesh[0] << " " << posRightMesh[1] << " " << posRightMesh[2]<< "\n";

}

void QTVTK_MSVEApplication::on_afterLeftNippleBtn_clicked()
{
	std::cout << "Position of left nipple on second mesh: " << posRightMesh[0] << " " << posRightMesh[1] << " " << posRightMesh[2]<< "\n";
	posAfterLeftNipple[0] = posRightMesh[0];
	posAfterLeftNipple[1] = posRightMesh[1];
	posAfterLeftNipple[2] = posRightMesh[2];	
	
}

void QTVTK_MSVEApplication::on_afterNavalBtn_clicked()
{
	std::cout << "Position of naval on second mesh: " << posRightMesh[0] << " " << posRightMesh[1] << " " << posRightMesh[2]<< "\n";
	posAfterNaval[0] = posRightMesh[0];
	posAfterNaval[1] = posRightMesh[1];
	posAfterNaval[2] = posRightMesh[2];
	
}
void QTVTK_MSVEApplication::readingFile()
{
	
	std::cout<< "\n" << "Reading stored landmarks..."<< "\n"<< "\n";
	
	QFile file("/Users/administrator/Documents/Git Projects/Scan Application/Output.txt");
	//QString path = QCoreApplication::applicationDirPath();
	//path.append("/Output.txt");
	//QFile file(path);
	
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		//qDebug() <<"Path not Found" << path;
		qDebug() <<"Path not Found";
	
	QTextStream in(&file);
	int count = 0;
	QStringList fields;
	
	while(!in.atEnd()) {
		QString line = in.readLine();     
		fields += line.split(",");  
		count++;
	}
	cout<< "List size:  "<< fields.size()<< "\n";
	
	for (int i = 0; i < 3; ++i){
		string str = fields.at(i).toLocal8Bit().constData();
		cout << str << endl;
		posAfterRightNipple[i] = atof(str.c_str());
	}
	for (int i = 0; i < 3; ++i){
		string str = fields.at(i+3).toLocal8Bit().constData();
		cout << str << endl;
		posAfterLeftNipple[i]=atof(str.c_str());
	}
	for (int i = 0; i < 3; ++i){
		string str = fields.at(i+6).toLocal8Bit().constData();
		cout << str << endl;
		posAfterNaval[i]=atof(str.c_str());
	}
	if (fields.size()==12) {
		for (int i = 0; i < 3; ++i){
			string str = fields.at(i+9).toLocal8Bit().constData();
			cout << str << endl;
			posAfterSternumNotch[i]=atof(str.c_str());
		}
	}
	
}
void QTVTK_MSVEApplication::on_registration3Marks_clicked()
{
	std::cout<< "\n" << "Registration using 3 landmarks..."<< "\n"<< "\n";
	
	//If user select to use stored landmarks, read file
	if(use_stored_landmarks)
	{
		readingFile();
	}
	//Else, save or overwrite the new landmarks
	else {
		int argc=3;
		writeFile(argc);
	}
	setting_landmarks();	
	registration();	
}

void QTVTK_MSVEApplication::writeFile(int landmarks_Num)
{
	QFile file("/Users/administrator/Documents/Git Projects/Scan Application/Output.txt");
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) return;
	QTextStream output(&file);
	
	
	double coordinates[3][3] = {{posAfterRightNipple[0], posAfterRightNipple[1], posAfterRightNipple[2]},
								{posAfterLeftNipple[0], posAfterLeftNipple[1], posAfterLeftNipple[2]},
								{posAfterNaval[0], posAfterNaval[1], posAfterNaval[2]}};
	
	for(int i = 0; i<3; i++)
		{
			for(int j = 0; j<3; j++)
			{
			output << coordinates[i][j]; //writing ith character of array in the file
			if(j<2)
				output<<",";
			else 
				output<<"\n";
		}
	}
	
	if(landmarks_Num ==4){
		for(int i = 0; i<3; i++)
		{
			output << posAfterSternumNotch[i]; //writing ith character of array in the file
			if(i<2)
				output<<",";
			else 
				output<<"\n";
		}	
	}
		
	
	file.close();	
}

void QTVTK_MSVEApplication::on_registration4Marks_clicked()
{
	std::cout<< "\n" << "Registration using 4 landmarks..."<< "\n"<< "\n";	
	
	//If user select to use stored landmarks, read file
	if(use_stored_landmarks)
	{
		readingFile();
	}
	//Else, save or overwrite the new landmarks
	else {
		int argc=4;
		writeFile(argc);
	}
	
	setting_landmarks();
	sourcePoints->InsertNextPoint(posBeforeSternumNotch);
	targetPoints->InsertNextPoint(posAfterSternumNotch);	
	
	registration();
	
}
void QTVTK_MSVEApplication::setting_landmarks()
{
	targetPoints->InsertNextPoint(posAfterNaval);
	targetPoints->InsertNextPoint(posAfterLeftNipple);
	targetPoints->InsertNextPoint(posAfterRightNipple);
	sourcePoints->InsertNextPoint(posBeforeRightNipple);
	sourcePoints->InsertNextPoint(posBeforeNaval);
	sourcePoints->InsertNextPoint(posBeforeLeftNipple);
}

void QTVTK_MSVEApplication::registration()	
{		
	// Setup the transform
	vtkSmartPointer<vtkLandmarkTransform> landmarkTransform = 
    vtkSmartPointer<vtkLandmarkTransform>::New();
	landmarkTransform->SetSourceLandmarks(sourcePoints);
	landmarkTransform->SetTargetLandmarks(targetPoints);
	//landmarkTransform->SetModeToSimilarity();
	//landmarkTransform->SetModeToAffine();
	landmarkTransform->SetModeToRigidBody();
	landmarkTransform->Update();
	
	vtkSmartPointer<vtkPolyData> source =
    vtkSmartPointer<vtkPolyData>::New();
	source = input1;
	
	vtkSmartPointer<vtkPolyData> target =
    vtkSmartPointer<vtkPolyData>::New();
	target = input2;
	
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	
	transformFilter->SetInputData(source);	
	transformFilter->SetTransform(landmarkTransform);
	transformFilter->Update();
	
	// Display the transformation matrix that was computed
	vtkMatrix4x4* mat = landmarkTransform->GetMatrix();
	std::cout << "Matrix: " << *mat << std::endl;
	
	// Visualize
	vtkSmartPointer<vtkPolyDataMapper> sourceMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
	sourceMapper->SetInputData(source);
	
	vtkSmartPointer<vtkActor> sourceActor =
    vtkSmartPointer<vtkActor>::New();
	sourceActor->SetMapper(sourceMapper);
	//sourceActor->GetProperty()->SetColor(0,1,0);
	//sourceActor->GetProperty()->SetPointSize(4);
	
	vtkSmartPointer<vtkPolyDataMapper> targetMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
	targetMapper->SetInputData(target);
	
	vtkSmartPointer<vtkActor> targetActor =
    vtkSmartPointer<vtkActor>::New();
	targetActor->SetMapper(targetMapper);
	//targetActor->GetProperty()->SetColor(1,0,0);
	//targetActor->GetProperty()->SetPointSize(4);
	
	vtkSmartPointer<vtkPolyDataMapper> solutionMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
	solutionMapper->SetInputConnection(transformFilter->GetOutputPort());
	
	vtkSmartPointer<vtkActor> solutionActor =
    vtkSmartPointer<vtkActor>::New();
	solutionActor->SetMapper(solutionMapper);
	solutionActor->GetProperty()->SetColor(0,0,1);
	solutionActor->GetProperty()->SetPointSize(2);
	
	vtkSmartPointer<vtkRenderer> ResultRenderer = 
	vtkSmartPointer<vtkRenderer>::New();
	
	// Add Actor to renderer
	ResultRenderer->AddActor(solutionActor);
	
	this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(ResultRenderer);
	this->qvtkWidgetLeft->update();
	ResultRenderer->GetActiveCamera()->SetPosition(0,-1,0);
	ResultRenderer->GetActiveCamera()->SetFocalPoint(0,0,0);
	ResultRenderer->ResetCamera();
	
	std::cout << "Calculating Difference map.."<< "\n"<< "\n";
	
	//Display Difference Map	
	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
	
	distanceFilter->SetInputConnection(0, transformFilter->GetOutputPort());
	distanceFilter->SetInputData(1, target);
	
	distanceFilter->Update();
	
	mapper2->SetInputConnection(distanceFilter->GetOutputPort());
	mapper2->SetScalarRange(
							distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0],
							distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[1]);
	
	vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
	actor2->SetMapper(mapper2);
	
	vtkSmartPointer<vtkRenderer> differenceRenderer = 
	vtkSmartPointer<vtkRenderer>::New();
	
	// Add Actor to renderer
	differenceRenderer->AddActor(actor2);
	
	this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(differenceRenderer);
	this->qvtkWidgetRight->update();
	differenceRenderer->GetActiveCamera()->SetPosition(0,-1,0);
	differenceRenderer->GetActiveCamera()->SetFocalPoint(0,0,0);
	differenceRenderer->ResetCamera();
	
}

void QTVTK_MSVEApplication::slotExit() 
{
	qApp->exit();
}
