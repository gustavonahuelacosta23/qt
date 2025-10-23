#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>
#include <QDial>
#include <QLineEdit>
#include <QPushButton>
#include <QLCDNumber>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void actualizarCaudalEntrada(int value);
    void actualizarCaudalSalidaCisterna(int value);
    void actualizarCaudalSalidaAux3(int value);
    void actualizarCaudalSalidaAux4(int value);
    void llenarCisterna();
    void distribuirSalida();
    bool puedeExtraer(double nivel, double capacidad);
    void actualizarLCD();
    void reiniciarCisterna();
    void guardarEstado();
    void cargarEstado();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    double nivelCisterna;
    double nivel3;
    double nivel4;
    double capacidadCisterna;
    double capacidad3;
    double capacidad4;
    double QmaxEntrada;
    double QmaxSalidaCisterna;
    double QmaxSalida3;
    double QmaxSalida4;
    double caudalEntrada;
    double caudalSalidaCisterna;
    double caudalSalida3;
    double caudalSalida4;
};

#endif // MAINWINDOW_H
