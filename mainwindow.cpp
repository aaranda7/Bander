#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QInputDialog"

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFileInfo>
#include <QDebug>
#include <QSqlError>
#include <QCryptographicHash>
#include <QByteArray>
#include <QStackedWidget>
#include <QRegExp>
#include <QMovie>
#include <QFileDialog>

#include <random>
#include <chrono>
#include <thread>

class Random {
private:
    std::mt19937 rng;
public:
    Random() {
        rng.seed(std::random_device()());
    }
    std::mt19937::result_type operator()(const std::mt19937::result_type min, const std::mt19937::result_type max) {
        std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max);
        return dist6(rng);
    }
};

class FileDialog : public QWidget{
private:
    QString filePath;
public:
    QString openFile(){
        this->filePath = QFileDialog::getOpenFileName(this, ("Abrir Archivo"), QDir::currentPath(), ("Images (*.png *.xpm *.jpg)"));
        return filePath;
    }
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    const QString dbFileName = "usuarios.db";
    QFileInfo dbFile(dbFileName);
    if(!dbFile.isFile()){
        printInformacion("No se puede abrir BD");
    }
    myDB = QSqlDatabase::addDatabase("QSQLITE"); // Tipo SQlite
    myDB.setDatabaseName(dbFileName); // El nombre de la BD
    if(!myDB.open()){
        printInformacion(myDB.lastError().text());
    }
    ui->setupUi(this);
    passwordSize = 6;
}

MainWindow::~MainWindow()
{
    // Cerrar coneccion a BD
    myDB.close();
    delete ui;
}

void MainWindow::printInformacion(const QString mensaje){
    QMessageBox msg;
    msg.setIcon(QMessageBox::Information);
    msg.setText(mensaje);
    msg.exec();
}
void MainWindow::printError(const QString error){
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setText(error);
    msg.exec();
}

void MainWindow::on_actionAcerca_de_Bander_triggered()
{
    const QString acerca = "Bander es una aplicacion para matchear solistas, bandas y representates!"
                           " Se requiere geolocalizacion para un matching ideal. Esta version esta siendo "
                           "testeada!";
    printInformacion(acerca);
}

void MainWindow::on_longitudPw_valueChanged(int arg1)
{
    passwordSize = arg1;
}

void MainWindow::on_mostrarPw_toggled(bool checked)
{
    if(!checked){
        ui->password->setEchoMode(QLineEdit::Password);
    } else{
        ui->password->setEchoMode(QLineEdit::Normal);
    }
}

void MainWindow::on_actionActualizar_triggered()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    const QString actualizar = "Se detecto la ultima version!";
    printInformacion(actualizar);
}

void MainWindow::on_loginButton_released()
{
    if(!myDB.open()){
         printError("Error conectarse BD");
         return;
    }
    const QString username = ui->lineEdit_2->text();
    const QString password = ui->lineEdit->text();
    this->usuario = username;

    QCryptographicHash hash(QCryptographicHash::Sha512); // SHA-512
    QByteArray pwByteArray =  password.toLocal8Bit();
    hash.addData(pwByteArray);

    QSqlQuery queryLogin;
    if(queryLogin.exec("SELECT * FROM usuarios WHERE username=\'" + username + "\' AND password=\'" + hash.result().toHex() + "\'")){
        if(queryLogin.next()){
            // Encontro el usuarios
            printInformacion("Login correcto!");
        } else{
            printError("Usuario/contraseña incorrecta!");
            return;
        }
    } else{
       printError("Error al buscar en BD");
       return;
    }

    // Mostrar pantalla de Matcheo
    // TODO
    ui->label->setText("Iniciado sesion como: " + queryTipoCuenta());
    ui->stackedWidget->setCurrentIndex(2); // 2 == Pag del usuario
}

void MainWindow::on_goToRegistrar_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_registerButton_clicked(){

    if(!myDB.open()){
        printError("Error al abrir BD para registrarse");
        return;
    }

    const QString username = ui->username->text();
    const QString password = ui->password->text();
    const QString email = ui->email->text();
    const QString tipoCuenta = ui->tipoCuenta->currentText();
    // La guardamos en atributos de la clase
    this->usuario = username;
    const QString fecha = ui->fecha->text();

    // Checkear el email que sea valido
    QRegExp regex("\\b[a-zA-Z0-9]+@[a-zA-Z]+\\.(com|net|ar)\\b"); // TODO: incluir mas dominios
    if(!regex.exactMatch(email)){
        printError("Email no valido!");
        return;
    }

    if(!username.size()){
        printError("Usuario/Contraseña en blanco!");
        return;
    } else if(password.size() < 5){
        printError("Contraseña tiene que tener 5+ caracteres");
        return;
    }

    // Hash the user pass
    QCryptographicHash hash(QCryptographicHash::Sha512); // SHA-512
    QByteArray pwByteArray =  password.toLocal8Bit();
    hash.addData(pwByteArray);

    //Check if user already exists
    // If not, create ...
    QSqlQuery queryRegister;
    if(queryRegister.exec("select username from usuarios where username=\'" + username + "\';")){
        if(!queryRegister.next()){
            if(!queryRegister.exec("insert into usuarios(username, password, email, cuenta, fecha) values(\'" + username + "\', \'" + hash.result().toHex() + "\',\'" + email + "\', \'" + tipoCuenta + "\', \'" + fecha + "\');")){
                printError("Error al crear usuario");
                return;
            } else {
                printInformacion("Usuario registrado!");
            }
        } else{
            printError("El usuario ya existe");
            return;
        }
    } else {
        printError("Error al ver si usuario existe o no!");
        return;
    }
    // Delete labels text
    ui->username->clear();
    ui->password->clear();
    ui->email->clear();
    ui->tipoCuenta->clear();
    ui->fecha->clear();
    ui->stackedWidget->setCurrentIndex(0);

}

void MainWindow::on_randomBut_clicked()
{
    QString password;
    Random ranNumber;
    ui->password->clear();
    for (int i = 0; i < passwordSize; i++) {
        password += static_cast<char>(ranNumber(33,126));
    }
    ui->password->insert(password);
}

void MainWindow::on_loginMostrarPw_clicked(bool checked)
{
    if(!checked){
        ui->lineEdit->setEchoMode(QLineEdit::Password);
    } else{
        ui->lineEdit->setEchoMode(QLineEdit::Normal);
    }
}


QString MainWindow::queryTipoCuenta(){
    QString retVal("");
    QSqlQuery queryRegister;
    if(queryRegister.exec("select username, cuenta from usuarios where username=\'" + this->usuario + "\';")){
        if(queryRegister.next()){
            retVal += queryRegister.value(1).toString();
        } else {
            printError("Error");
        }
    }

    return retVal;
}

void MainWindow::on_ePerfil_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(2);
}

void MainWindow::on_Chats_clicked()
{

}

void MainWindow::on_Matchear_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);
    QMovie *movie = new QMovie(":/gifs/loading_L.gif");
    if (!movie->isValid())
        {
         printError(movie->lastErrorString());
        }
    ui->displayLabel->setMovie(movie);
    movie->start();
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //delete movie;
}

void MainWindow::on_CerrarSesion_clicked()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // ir al menu principal
    ui->stackedWidget->setCurrentIndex(0); // Menu de Logeo
}

void MainWindow::on_cancelarMatch_released()
{
    ui->stackedWidget_2->setCurrentIndex(0);
}

void MainWindow::on_pushButton_clicked()
{
    FileDialog abrirFoto;
    const QString filePath = abrirFoto.openFile();
    qDebug() << filePath;
}

void MainWindow::on_pushButton_2_released()
{
    ui->stackedWidget_2->setCurrentIndex(0);
}
