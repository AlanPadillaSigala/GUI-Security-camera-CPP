#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv/cv.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/video/background_segm.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgcodecs.hpp>
#include<QDebug> //Similar a la funcion cout
#include<opencv2/objdetect.hpp>
//--------Librerias para el correo-----------------------
#include<QTimer>
#include<QFile>
#include<QDir>
#include<QDirIterator>
#include<QProcess>
//#include<QtConcurrent>
#include<QThread>
//#include<stdlib.h>
//--------Librerias para la base de datos -------------------
#include<QtSql/QSqlDatabase>
#include<QtSql/QSql>
#include<QtSql/QSqlQuery>
#include<QtSql/QSqlError>
#include<QtNetwork>
#include<QDate>
#include<QDateTime>
#include<QTime>
//---------------------------------------------------------------

int contador = 0;

QString nombreImagen; //base datos

using namespace cv;
CascadeClassifier deteccionpersonas;

//VideoCapture camara(0); //Usar camara PC
VideoCapture camara("http://192.168.111.111:8080/video"); //Ip borrada

Mat imagen1;
Mat imagen1_chica;
Mat imagen_gris;
Mat imagen_error(500,400, CV_8UC3, Scalar(0,0,255));
Mat imagenFija;

int Cobjetos = 0;
bool estaEditandose = false; //base datos
bool detecta = false;
bool disparo = false;

bool graba = false;
bool grabarv = false;
VideoWriter videoGrabado;
bool disparo2 = false;
//bool disparo3 = false;


void MainWindow::funcionCronometro(){

camara >> imagen1;

QString fecha = QDate::currentDate().toString("dd-MM-yyyy"); //nombre imagen
QString hora = QTime::currentTime().toString("-hh-mm-ss"); //nombre imagen
nombreImagen = fecha+hora+".jpg";

if (ui->checkBox_2->isChecked()) detecta = true;   //activacion manual
//if (!ui->checkBox_2->isChecked()) detecta = false;

if (ui->checkBox_3->isChecked()) graba = true;     //activacion manual
if (!ui->checkBox_3->isChecked()) graba = false;

if (!ui->checkBox->isChecked()){ //Seleccion de camara

    contador++;
    ui->lcdNumber->display(contador);
            QDirIterator it("/home/alan/Maildir/process/landing", QStringList() << "*.alan-HP-Laptop-15-bs0xx", QDir::Files, QDirIterator::Subdirectories);
                   while (it.hasNext()) { //correo
                         QString filename = it.next();
                         QFileInfo file(filename);
                         QFile f(filename);
                         if (file.isDir()) { //Verificar si el archivo es un directorio
                             continue;
                         }

                         if (file.isFile()) { //Si se encuentra un archivo...

                                    if (!f.open(QFile::ReadOnly | QFile::Text)) break;
                                    QTextStream in(&f);
                                    QString texto;
                                    texto = in.readAll();
                                    texto = texto.toLower();
                                    //qDebug() << texto;

									// --- * * * Deteccion de funciones por medio de email * * * ---
                                    int deteccion = texto.indexOf("det");	//Activar la deteccion de personas
                                    int nodetect = texto.indexOf("nodet");	//Desactivar deteccion
                                    int foto = texto.indexOf("foto");		//Tomar foto
                                    int grabacion = texto.indexOf("grabar");//Iniciar la Grabacion
                                    int nograb= texto.indexOf("nog");		//Detener la Grabacion

                                    if (deteccion > 0){ //deteccion por correo
                                        detecta=true;
                                    }//fin
                                    if (nodetect > 0){ //deteccion por correo
                                        detecta=false;
                                    }//fin
                                    if(foto > 0){ //Tomar foto
                                        //captura = 1;
                                        camara >> imagen1;
                                        imwrite("imagen.jpg", imagen1);
                                        QString comando = "mpack -s subject imagen.jpg alan@alumnos.udg.mx"; //Correo borrado
                                        system(comando.toUtf8().constData());
                                    } //fin
                                    if (grabacion > 0){//Grabar video
                                        graba = true;
                                    }//fin
                                    if (nograb > 0){
                                        graba = false;
                                    }
                         f.close();
                         f.remove();
                         } }
//--------------------------------------------------------------------------------------------------
if(graba == true){ //activa grabacion
        if (disparo2 == false){
            Mat IMAGENpequena;
            cv::resize(imagen1,IMAGENpequena,Size(640,360));  //Paso # 3

                    //Guardar video
                    videoGrabado << imagen1;
                    Size S = Size((int) camara.get(CV_CAP_PROP_FRAME_WIDTH),
                                  (int) camara.get(CV_CAP_PROP_FRAME_HEIGHT));
                    int ex = static_cast<int>(camara.get(CV_CAP_PROP_FOURCC));
                    videoGrabado.open("../video.mp4" , ex, camara.get(CV_CAP_PROP_FPS),S, true);

            QImage qImage = Mat2QImage(IMAGENpequena);
            QPixmap pixmap = QPixmap::fromImage(qImage);
            ui->label_2->clear();
            ui->label_2->setPixmap(pixmap);
            disparo2 = true;
        }
}
//--------------------------------------------------------------------------------------------------

                   //Se realiza el procesamiento y deteccion de personas
                   cv::resize(imagen1, imagen1_chica, Size(640,360),0,0,INTER_LINEAR);

                   cvtColor(imagen1_chica, imagen_gris, CV_BGR2GRAY);

                   equalizeHist(imagen_gris, imagen_gris);

                   std::vector<Rect> objeto1_Encontrados; //variable para guardar las detecciones

                   if (detecta == true){ //activacion desactivacion de la deteccion
                   deteccionpersonas.detectMultiScale(imagen_gris, objeto1_Encontrados, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(90,90));
                   }

                   if(objeto1_Encontrados.size() == 0){
                       Cobjetos = 0;
                   }

                   for(int i = 0; i<(int)objeto1_Encontrados.size(); i++){ //dibujar una elipse
                       Point centroRostro(objeto1_Encontrados[i].x+objeto1_Encontrados[i].width/2, objeto1_Encontrados[i].y+objeto1_Encontrados[i].height/2);
                       ellipse(imagen1_chica, centroRostro, Size(objeto1_Encontrados[i].width/2, objeto1_Encontrados[i].height/2),0,0,360,Scalar(255,0,0),4,8,0);
                       Cobjetos++;
                   }

                   if (Cobjetos > 10){ //Aplicar cambios
                       if (Cobjetos > 11) Cobjetos=0; //detectar nuevamente
                       putText(imagen1_chica, "Intruso", Point(40,40), FONT_HERSHEY_COMPLEX, 0.5, CV_RGB(255,0,0), 2);
                       imwrite("imagen.jpg", imagen1_chica);
                       QString comando = "mpack -s subject imagen.jpg alan@alumnos.udg.mx"; //Correo borrado
                       system(comando.toUtf8().constData());

                   imwrite(nombreImagen.toUtf8().constData(), imagen1_chica);
                   QSqlDatabase baseDatos = QSqlDatabase::addDatabase("QMYSQL");
                   baseDatos.setHostName("localhost");
                   baseDatos.setPort(3306);
                   baseDatos.setDatabaseName("CamaraSeguridad");
                   baseDatos.setUserName("Insert"); //Username borrado
                   baseDatos.setPassword("Insert");	//Contrasena borrada 

                   if(!baseDatos.open()){
                       qDebug() << "Error al abrir la base de datos";
                   }

                   else{
                       QSqlDatabase db = baseDatos;
                       if(estaEditandose){
                            estaEditandose = false;
                            imwrite(ui->comboBox_2->currentText().toUtf8().constData(), imagen1);
                       }
                       else{
                           QString comandoMYSQL = "INSERT INTO HISTORIAL(CAPTURAS) VALUES (?)";
                           QSqlQuery comando;
                           comando.prepare(comandoMYSQL);
                           comando.addBindValue(nombreImagen);
                           comando.exec();
                       }

                   }
                   baseDatos.close();
                }//fin if de Cobjetos

                   QImage imagenQT = Mat2QImage(imagen1_chica);
                   QPixmap imagenMapa = QPixmap::fromImage(imagenQT);
                   ui->label_2->clear();
                   ui->label_2->setPixmap(imagenMapa);
}
//------------------------------------------------------------------------------------------------------------
if (ui->checkBox->isChecked()){ //seleccion de la Base de datos
     if (disparo == false){
         detecta = false;//dasactiva deteccion (Rendimiento)
     QSqlDatabase baseDatos = QSqlDatabase::addDatabase("QMYSQL");
     baseDatos.setHostName("localhost");
     baseDatos.setPort(3306);
     baseDatos.setDatabaseName("CamaraSeguridad");
     baseDatos.setUserName("Insert"); //Username borrado
     baseDatos.setPassword("Insert"); //Contrasena borrada

      if(!baseDatos.open()){
         qDebug() << "Error al abrir la base de datos";
      }
      else{
         QString comandoMYSQL = "SELECT * FROM HISTORIAL ORDER BY ID DESC";
         QSqlQuery comando = baseDatos.exec(comandoMYSQL);
         int contadorFila = 0;
         ui->comboBox->clear();
         ui->comboBox_2->clear();

         while(comando.next()) {
             contadorFila ++;
             int indiceTabla = comando.value(0).toInt();
             QString nombreImagenTabla = comando.value(1).toString();
             ui->comboBox->addItem(nombreImagenTabla);
             ui->comboBox_2->addItem(QString::number(indiceTabla));
             }
             qDebug() << "Se encontraron: " << contadorFila << " filas en la tabla HISTORIAL" << endl;
       }
     baseDatos.close();
     disparo=true;
     }
}
}
//------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Grabacion
    QPixmap pixmap("../Imagenes/grabar-desactivo.png");
    QIcon ButtonIcon(pixmap);
    ui->pushButton->setIcon(ButtonIcon);
    ui->pushButton->setIconSize(pixmap.rect().size());

    pixmap.load("../Imagenes/stop.png");
    ButtonIcon.addPixmap(pixmap);
    ui->pushButton_2->setIcon(ButtonIcon);
    ui->pushButton_2->setIconSize(pixmap.rect().size());

    //Base datos
    ui->comboBox_2->setVisible(false);

    //Pre-cargar al clasificador
    deteccionpersonas.load("../HAAR-PARA-PARTE-SUPERIOR-DEL-CUERPO.xml");

    QTimer *cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()), this, SLOT(funcionCronometro()));
    cronometro->start(30);
}

MainWindow::~MainWindow()
{
    //Se borra la ventana
    delete ui;
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    ui->comboBox_2->setCurrentIndex(index);
    Mat img1 = imread(ui->comboBox->currentText().toUtf8().constData());
    Mat img2;
    if(!img1.empty()){
    cv::resize(img1, img2, Size(640,360),0,0,INTER_LINEAR);
    QImage imagenQT = Mat2QImage(img2);
     QPixmap imagenMapa = QPixmap::fromImage(imagenQT);
     ui->label_2->clear();
    ui->label_2->setPixmap(imagenMapa);
    }
}

void MainWindow::on_pushButton_clicked(bool checked)
{
    grabarv = checked;
    if(checked){
        //Cambiar a imagen roja
        QPixmap pixmap("../Imagenes/grabar-activo.png");
        QIcon ButtonIcon(pixmap);
        ui->pushButton->setIcon(ButtonIcon);
        ui->pushButton->setIconSize(pixmap.rect().size());
    }
    else{
        QPixmap pixmap("../Imagenes/grabar-desactivo.png");
        QIcon ButtonIcon(pixmap);
        ui->pushButton->setIcon(ButtonIcon);
        ui->pushButton->setIconSize(pixmap.rect().size());
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    videoGrabado.release();
    grabarv = false;
    ui->pushButton->setChecked(false);

    //Cambiamos la imagen a la gris
    QPixmap pixmap("../Imagenes/grabar-desactivo.png");
    QIcon ButtonIcon(pixmap);
    ui->pushButton->setIcon(ButtonIcon);
    ui->pushButton->setIconSize(pixmap.rect().size());
}
