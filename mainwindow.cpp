//  Author      : Черняев Андрей
//  Description : Многопоточное приложение с передачей данных из нити обработки в нить формы по указателю
// с использованием QByteArray вместо uint8_t[]
// Полное описание https://habr.com/ru/post/485324/
#include "mainwindow.h"
#include "ui_mainwindow.h"

Worker::Worker(){
    this->timerDeviceRead = new QTimer();
    connect(Worker::timerDeviceRead, &QTimer::timeout, this, &Worker::updateUSBDataCallback);
    this->timerDeviceRead->start();
}

void Worker::updateUSBDataCallback(){
    QByteArray buf("I like slow USB HID");
    emit GuiUpdatePlease(buf);
    //Этот код окажет влияние при типе соединения QueuedConnection между GuiUpdatePlease и GuiUpdateCallback
    //вызывая грязное чтения
    for(int i =0;i<buf.size();i++){
        buf[i]=i+'0';
        if (i+'0' > 250) i=0;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    thread = new QThread;
    Worker *worker = new Worker;
    worker->moveToThread(thread); //The object cannot be moved if it has a parent.
    //Попробуйте вариант соединения Qt::QueuedConnection
    connect(worker, &Worker::GuiUpdatePlease, this, &MainWindow::GuiUpdateCallback, Qt::BlockingQueuedConnection);
    thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GuiUpdateCallback(QByteArray arr){
    for(volatile uint64_t i=0;i<20000000UL;i++);  //имитируем задержкой, что arrptr очень сложно и долго выводится в GUI (обостряем грязное чтение)
    QString DataAsString = QString(arr);
    ui->textEdit->setText(DataAsString);
}
