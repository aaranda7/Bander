#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAcerca_de_Bander_triggered();

    void on_longitudPw_valueChanged(int arg1);

    void on_mostrarPw_toggled(bool checked);

    void on_actionActualizar_triggered();

    void on_loginButton_released();

    void on_goToRegistrar_clicked();
    void on_registerButton_clicked();

    void on_randomBut_clicked();

    void on_loginMostrarPw_clicked(bool checked);

    void on_ePerfil_clicked();

    void on_Chats_clicked();

    void on_Matchear_clicked();

    void on_CerrarSesion_clicked();

    void on_cancelarMatch_released();

    void on_pushButton_clicked();

    void on_pushButton_2_released();

private:
    Ui::MainWindow *ui;
    QSqlDatabase myDB;

    int passwordSize;

    void printInformacion(const QString mensaje);
    void printError(const QString error);

    QString usuario;
    QString queryTipoCuenta();

};

#endif // MAINWINDOW_H
