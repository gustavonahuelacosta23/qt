#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <algorithm>
#include <QFile>

static inline double extraible(double nivel, double capacidad) {
    return std::max(0.0, nivel - 0.1 * capacidad);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    nivelCisterna = nivel3 = nivel4 = 0.0;
    capacidadCisterna = 100.0;
    capacidad3 = 50.0;
    capacidad4 = 50.0;

    QmaxEntrada = 30.0;
    QmaxSalidaCisterna = 20.0;
    QmaxSalida3 = 10.0;
    QmaxSalida4 = 10.0;

    caudalEntrada = caudalSalidaCisterna = caudalSalida3 = caudalSalida4 = 0.0;

    ui->dial ->setRange(0,100);
    ui->dial2->setRange(0,100);
    ui->dial3->setRange(0,100);
    ui->dial4->setRange(0,100);

    ui->cisterna->setRange(0, capacidadCisterna);
    ui->auxiliar3->setRange(0, capacidad3);
    ui->auxiliar4->setRange(0, capacidad4);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::llenarCisterna);
    timer->start(1000);

    connect(ui->dial,  &QDial::valueChanged, this, &MainWindow::actualizarCaudalEntrada);
    connect(ui->dial2, &QDial::valueChanged, this, &MainWindow::actualizarCaudalSalidaCisterna);
    connect(ui->dial3, &QDial::valueChanged, this, &MainWindow::actualizarCaudalSalidaAux3);
    connect(ui->dial4, &QDial::valueChanged, this, &MainWindow::actualizarCaudalSalidaAux4);

    connect(ui->botonReiniciar, &QPushButton::clicked, this, &MainWindow::reiniciarCisterna);
    connect(ui->botonGuardar,   &QPushButton::clicked, this, &MainWindow::guardarEstado);
    connect(ui->botonCargar,    &QPushButton::clicked, this, &MainWindow::cargarEstado);

    ui->Qmax->setText(QString::number(QmaxEntrada));
    ui->Qmax2->setText(QString::number(QmaxSalidaCisterna));
    ui->Qmax3->setText(QString::number(QmaxSalida3));
    ui->Qmax4->setText(QString::number(QmaxSalida4));

    ui->capacidad->setText(QString::number(capacidadCisterna));
    ui->capacidad3->setText(QString::number(capacidad3));
    ui->capacidad4->setText(QString::number(capacidad4));

    auto configurarQmax = [this](QLineEdit *edit, double &variable) {
        connect(edit, &QLineEdit::editingFinished, this, [this, edit, &variable]() {
            bool ok;
            double val = edit->text().toDouble(&ok);
            if (ok && val >= 0) {
                variable = val;
                edit->setStyleSheet("");
                if (edit == ui->Qmax)  actualizarCaudalEntrada(ui->dial->value());
                if (edit == ui->Qmax2) actualizarCaudalSalidaCisterna(ui->dial2->value());
                if (edit == ui->Qmax3) actualizarCaudalSalidaAux3(ui->dial3->value());
                if (edit == ui->Qmax4) actualizarCaudalSalidaAux4(ui->dial4->value());
            } else {
                edit->setStyleSheet("background-color: #ffcccc;");
            }
        });
    };

    configurarQmax(ui->Qmax,  QmaxEntrada);
    configurarQmax(ui->Qmax2, QmaxSalidaCisterna);
    configurarQmax(ui->Qmax3, QmaxSalida3);
    configurarQmax(ui->Qmax4, QmaxSalida4);

    auto configurarCapacidad = [this](QLineEdit *edit, double &capacidad, QProgressBar *barra) {
        connect(edit, &QLineEdit::editingFinished, this, [this, edit, &capacidad, barra]()  {
            bool ok;
            double val = edit->text().toDouble(&ok);
            if (ok && val > 0) {
                capacidad = val;
                barra->setRange(0, capacidad);
                if (barra == ui->cisterna)  nivelCisterna = std::min(nivelCisterna, capacidadCisterna);
                if (barra == ui->auxiliar3) nivel3        = std::min(nivel3,        capacidad3);
                if (barra == ui->auxiliar4) nivel4        = std::min(nivel4,        capacidad4);
                edit->setStyleSheet("");

                ui->cisterna->setValue(nivelCisterna);
                ui->auxiliar3->setValue(nivel3);
                ui->auxiliar4->setValue(nivel4);
                actualizarLCD();
            } else {
                edit->setStyleSheet("background-color: #ffcccc;");
            }
        });
    };

    configurarCapacidad(ui->capacidad,  capacidadCisterna, ui->cisterna);
    configurarCapacidad(ui->capacidad3, capacidad3,        ui->auxiliar3);
    configurarCapacidad(ui->capacidad4, capacidad4,        ui->auxiliar4);

    ui->lcdsalida->display(caudalEntrada);
    ui->lcd2->display(caudalSalidaCisterna);
    ui->lcdsalida1->display(caudalSalida3);
    ui->lcdsalida2->display(caudalSalida4);
    actualizarLCD();
}

MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::puedeExtraer(double nivel, double capacidad)
{
    return nivel > 0.1 * capacidad;
}

void MainWindow::actualizarLCD()
{
    ui->lcd->display(nivelCisterna);
    ui->lcd3->display(nivel3);
    ui->lcd4->display(nivel4);
}

void MainWindow::actualizarCaudalEntrada(int value)
{
    caudalEntrada = (value / 100.0) * QmaxEntrada;
    ui->lcdsalida->display(caudalEntrada);
}

void MainWindow::actualizarCaudalSalidaCisterna(int value)
{
    caudalSalidaCisterna = (value / 100.0) * QmaxSalidaCisterna;
    ui->lcd2->display(caudalSalidaCisterna);
}

void MainWindow::actualizarCaudalSalidaAux3(int value)
{
    caudalSalida3 = (value / 100.0) * QmaxSalida3;
    ui->lcdsalida1->display(caudalSalida3);
}

void MainWindow::actualizarCaudalSalidaAux4(int value)
{
    caudalSalida4 = (value / 100.0) * QmaxSalida4;
    ui->lcdsalida2->display(caudalSalida4);
}


void MainWindow::llenarCisterna()
{

    if (nivelCisterna >= capacidadCisterna)
        caudalEntrada = 0;


    nivelCisterna += caudalEntrada / 3600.0;


    distribuirSalida();


    if (!puedeExtraer(nivelCisterna, capacidadCisterna)) {
        caudalSalidaCisterna = 0;
        ui->dial2->setValue(0);
    }
    if (!puedeExtraer(nivel3, capacidad3)) {
        caudalSalida3 = 0;
        ui->dial3->setValue(0);
    }
    if (!puedeExtraer(nivel4, capacidad4)) {
        caudalSalida4 = 0;
        ui->dial4->setValue(0);
    }


    nivelCisterna = std::clamp(nivelCisterna, 0.0, capacidadCisterna);
    nivel3 = std::clamp(nivel3, 0.0, capacidad3);
    nivel4 = std::clamp(nivel4, 0.0, capacidad4);


    ui->cisterna->setValue(nivelCisterna);
    ui->auxiliar3->setValue(nivel3);
    ui->auxiliar4->setValue(nivel4);

    actualizarLCD();
}


void MainWindow::distribuirSalida()
{

    if (puedeExtraer(nivel3, capacidad3)) {
        double egreso3 = std::min(extraible(nivel3, capacidad3), caudalSalida3 / 3600.0);
        nivel3 = std::max(0.0, nivel3 - egreso3);
    } else {
        caudalSalida3 = 0;
        ui->dial3->setValue(0);
    }

    if (puedeExtraer(nivel4, capacidad4)) {
        double egreso4 = std::min(extraible(nivel4, capacidad4), caudalSalida4 / 3600.0);
        nivel4 = std::max(0.0, nivel4 - egreso4);
    } else {
        caudalSalida4 = 0;
        ui->dial4->setValue(0);
    }


    const bool aux3Habilitado = ui->checkAux3->isChecked();
    const bool aux4Habilitado = ui->checkAux4->isChecked();

    if (!puedeExtraer(nivelCisterna, capacidadCisterna)) {
        caudalSalidaCisterna = 0;
        ui->dial2->setValue(0);
    } else {
        double maxDesdeCisterna = extraible(nivelCisterna, capacidadCisterna);
        double pedido = caudalSalidaCisterna / 3600.0;
        double disponible = std::min(maxDesdeCisterna, pedido);

        const bool aux3Lleno = nivel3 >= capacidad3;
        const bool aux4Lleno = nivel4 >= capacidad4;

        if (disponible > 0.0 && (aux3Habilitado || aux4Habilitado) &&
            (!(aux3Habilitado && aux3Lleno) || !(aux4Habilitado && aux4Lleno))) {

            auto transferir = [&](double& destino, double cap, double cantidad) {
                double espacio = cap - destino;
                double mov = std::min(cantidad, std::max(0.0, espacio));
                destino       += mov;
                nivelCisterna -= mov;
                return cantidad - mov;
            };

            if (aux3Habilitado && aux4Habilitado && !aux3Lleno && !aux4Lleno) {
                double mitad = disponible / 2.0;
                double sobra = transferir(nivel3, capacidad3, mitad);
                double resta = mitad + sobra;
                transferir(nivel4, capacidad4, resta);
            } else if (aux3Habilitado && !aux3Lleno) {
                transferir(nivel3, capacidad3, disponible);
            } else if (aux4Habilitado && !aux4Lleno) {
                transferir(nivel4, capacidad4, disponible);
            }
        }
    }

    nivelCisterna = std::clamp(nivelCisterna, 0.0, capacidadCisterna);
    nivel3        = std::clamp(nivel3,        0.0, capacidad3);
    nivel4        = std::clamp(nivel4,        0.0, capacidad4);
}


void MainWindow::reiniciarCisterna()
{
    nivelCisterna = nivel3 = nivel4 = 0;
    ui->cisterna->setValue(0);
    ui->auxiliar3->setValue(0);
    ui->auxiliar4->setValue(0);
    ui->dial->setValue(0);
    ui->dial2->setValue(0);
    ui->dial3->setValue(0);
    ui->dial4->setValue(0);

    ui->lcdsalida->display(0);
    ui->lcd2->display(0);
    ui->lcdsalida1->display(0);
    ui->lcdsalida2->display(0);

    actualizarLCD();

    statusBar()->showMessage("Reiniciado correctamente.", 3000);
    QMessageBox::information(this, "Reiniciar", "La simulación se reinició correctamente.");
}

void MainWindow::guardarEstado()
{
    QString filename = QFileDialog::getSaveFileName(this, "Guardar estado", "", "Archivos de texto (*.txt)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << nivelCisterna << " " << nivel3 << " " << nivel4 << " "
            << capacidadCisterna << " " << capacidad3 << " " << capacidad4 << " "
            << QmaxEntrada << " " << QmaxSalidaCisterna << " "
            << QmaxSalida3 << " " << QmaxSalida4 << " "
            << ui->dial->value()  << " "
            << ui->dial2->value() << " "
            << ui->dial3->value() << " "
            << ui->dial4->value() << " "
            << (ui->checkAux3->isChecked() ? 1 : 0) << " "
            << (ui->checkAux4->isChecked() ? 1 : 0) << "\n";
        file.close();

        statusBar()->showMessage("Archivo guardado correctamente.", 3000);
        QMessageBox::information(this, "Guardar", "El estado se guardó correctamente.");
    } else {
        QMessageBox::warning(this, "Guardar", "No se pudo abrir el archivo para escribir.");
    }
}

void MainWindow::cargarEstado()
{
    QString filename = QFileDialog::getOpenFileName(this, "Cargar estado", "", "Archivos de texto (*.txt)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        in >> nivelCisterna >> nivel3 >> nivel4
            >> capacidadCisterna >> capacidad3 >> capacidad4
            >> QmaxEntrada >> QmaxSalidaCisterna
            >> QmaxSalida3 >> QmaxSalida4;

        int d1 = ui->dial->value();
        int d2 = ui->dial2->value();
        int d3 = ui->dial3->value();
        int d4 = ui->dial4->value();
        int c3 = ui->checkAux3->isChecked() ? 1 : 0;
        int c4 = ui->checkAux4->isChecked() ? 1 : 0;

        if (!in.atEnd()) in >> d1;
        if (!in.atEnd()) in >> d2;
        if (!in.atEnd()) in >> d3;
        if (!in.atEnd()) in >> d4;
        if (!in.atEnd()) in >> c3;
        if (!in.atEnd()) in >> c4;

        file.close();

        ui->cisterna->setRange(0, capacidadCisterna);
        ui->auxiliar3->setRange(0, capacidad3);
        ui->auxiliar4->setRange(0, capacidad4);

        ui->cisterna->setValue(nivelCisterna);
        ui->auxiliar3->setValue(nivel3);
        ui->auxiliar4->setValue(nivel4);

        ui->Qmax->setText(QString::number(QmaxEntrada));
        ui->Qmax2->setText(QString::number(QmaxSalidaCisterna));
        ui->Qmax3->setText(QString::number(QmaxSalida3));
        ui->Qmax4->setText(QString::number(QmaxSalida4));

        ui->capacidad->setText(QString::number(capacidadCisterna));
        ui->capacidad3->setText(QString::number(capacidad3));
        ui->capacidad4->setText(QString::number(capacidad4));

        ui->checkAux3->setChecked(c3 != 0);
        ui->checkAux4->setChecked(c4 != 0);

        ui->dial->setValue(d1);
        ui->dial2->setValue(d2);
        ui->dial3->setValue(d3);
        ui->dial4->setValue(d4);

        actualizarLCD();

        statusBar()->showMessage("Archivo cargado correctamente.", 3000);
        QMessageBox::information(this, "Cargar", "El archivo se cargó correctamente.");
    } else {
        QMessageBox::warning(this, "Cargar", "No se pudo abrir el archivo para lectura.");
    }
}
