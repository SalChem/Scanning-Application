#include <QApplication>
#include "QTVTK_MSVEApplication.h"
 
int main( int argc, char** argv )
{
  // QT Stuff
  QApplication app( argc, argv );
 
  QTVTK_MSVEApplication MSVEApplication;
  MSVEApplication.show();
 
  return app.exec();
}