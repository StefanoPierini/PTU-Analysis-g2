#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "measure.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>
//#include <iostream>


static int g2width;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QValidator *integervalidator= new QIntValidator();
    QValidator *doublevalidator= new QDoubleValidator();
    ui->lineEdit_Threshold->setValidator(integervalidator);
    ui->lineEdit_TimeLimit->setValidator(integervalidator);
    ui->lineEdit_IntTime->setValidator(doublevalidator);
    curveg2 = new QwtPlotCurve("");
    //    connect(ui->pushButton_Analize, SIGNAL(clicked()),SLOT(on_pushButton_Analize_clicked()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Data file"), "/home/pierinis/Documenti/Dottorato/DatiParigi", tr("PTU data files (*.ptu)"));
    ui->textEdit_fileName->setText(fileName);
}

void MainWindow::on_pushButton_Analize_clicked()
{
    //    QMessageBox::StandardButton reply;
    //    reply = QMessageBox::question(this, "Save", "Do you want to save your changes?",
    //    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    //    Measure *M= new Measure(ui->textEdit_fileName->toPlainText().toStdString());
    //    M.readHeader();
    ui->pushButton_Analize->setDisabled(true);
    QString text=ui->pushButton_Analize->text();
    ui->pushButton_Analize->setText("Analisis in progress");
    int MC;
    bool flag_normalization=false;
    int sogliaGlob=0;
    int g2width=10;
    double altAt=0;
    double intTime=0.01;

    if(ui->radioButton_ch1->isChecked()){
        MC=1;
    }else if(ui->radioButton_ch2->isChecked()){
        MC=2;
    }else{
        QMessageBox msgBox;
        msgBox.setText("Please select Channel 1 or Channel 2");
        msgBox.exec();
        goto end;
    }
    if(ui->checkBox_normalize->isChecked()){
        QMessageBox msgBox;
        msgBox.setText("User required normalization");
        msgBox.setInformativeText("this may require some time. Do you want to proceed?");
        msgBox.setStandardButtons(QMessageBox::No|QMessageBox::Yes);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Yes:
            flag_normalization=true;
            break;
        case QMessageBox::No:
            goto end;
        }
    }
    if(ui->checkBox_Threshold->isChecked()){
        if(ui->lineEdit_Threshold->text().toInt()<=0){
            QMessageBox msgBox;
            msgBox.setText("Please select a valid value for threshold,\nor disable it");
            msgBox.setInformativeText("valid values are all positive integers");
            msgBox.exec();
            goto end;
        }
        sogliaGlob=ui->lineEdit_Threshold->text().toInt();

    }
    if(ui->lineEdit_PulseNumber->text().toInt()>1){
        g2width=ui->lineEdit_PulseNumber->text().toInt();
    }else
    {
        QMessageBox msgBox;
        msgBox.setText("Number of pulses must be at least 2");
        msgBox.exec();
        goto end;
    }
    if(ui->checkBox_TimeLimit->isChecked()){
        if(ui->lineEdit_TimeLimit->text().toDouble()<=0){
            QMessageBox msgBox;
            msgBox.setText("Time limit, if selected, must be positive");
            msgBox.setInformativeText("please select a valid value or disable it");
            msgBox.exec();
            goto end;
        }
        altAt=ui->lineEdit_TimeLimit->text().toDouble();

    }
    if(ui->lineEdit_IntTime->text().toDouble()>0){
        intTime=ui->lineEdit_IntTime->text().toDouble();
    }else{
        QMessageBox msgBox;
        msgBox.setText("Integration time must bigger than 0");
        msgBox.exec();
        goto end;
    }

    {
        int binNum=50;
        Measure *M= new Measure(ui->textEdit_fileName->toPlainText().toStdString(), MC, flag_normalization, sogliaGlob, binNum, g2width, altAt, intTime);
        //      Measure *M= new Measure(ui->textEdit_fileName->toPlainText().toStdString(), int MC, int flag_normalization, int sogliaGlob, int binNum, double altAt,int sON);
        delete M;
    }

end:
    ui->pushButton_Analize->setDisabled(false);
    ui->pushButton_Analize->setText(text);
    return;
}

void MainWindow::on_pushButton_graphg2_clicked() {
    ui->qwtPlot_g2->setTitle("g2 plot");
    std::string FileName = ui->textEdit_fileName->toPlainText().toStdString();
    boost::filesystem::path File_name=boost::filesystem::path(FileName);
    boost::filesystem::path File_out=Append_to_name(File_name,"out");
    boost::filesystem::path File_int=Append_to_name(File_name,"int");
    boost::filesystem::path File_life=Append_to_name(File_name,"life");
    boost::filesystem::path File_g2_far=Append_to_name(File_name,"g2_far");
    boost::filesystem::path File_g2_norm=Append_to_name(File_name,"g2_norm");
    File_out.replace_extension(".dat");
    File_int.replace_extension(".dat");
    File_life.replace_extension(".dat");
    File_g2_far.replace_extension(".dat");
    File_g2_norm.replace_extension(".dat");

//    if(!boost::filesystem::exists(File_out)){
//        goto end;
//    }
    {
        curveg2->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_out.c_str()));

            //        QwtPlotCurve *curveg2 = new QwtPlotCurve("");

            curveg2->setPen(Qt::blue,4);
            QVector<double> datax= QVector<double>::fromStdVector(datas[1]);
            QVector<double> datay= QVector<double>::fromStdVector(datas[3]);
            curveg2->setSamples(datax,datay);
            curveg2->attach(ui->qwtPlot_g2);
            ui->qwtPlot_g2->setAxisTitle(QwtPlot::Axis::xBottom,"time (s)");
            ui->qwtPlot_g2->setAxisTitle(QwtPlot::Axis::yLeft,"counts");
            ui->qwtPlot_g2->replot();
        } catch (const char* msg) {
            QMessageBox msgBox;
            msgBox.setText("An exception occurred:");
            msgBox.setInformativeText(msg);
            msgBox.exec();
            goto end;
        }
    }
    //        curveg2=curve1;
end:
    nullptr;
}


void MainWindow::on_pushButton_pushButton_g2far_clicked(){
    ui->qwtPlot_g2->setTitle("g2 plot");
    std::string FileName = ui->textEdit_fileName->toPlainText().toStdString();
    boost::filesystem::path File_name=boost::filesystem::path(FileName);
    boost::filesystem::path File_out=Append_to_name(File_name,"out");
    boost::filesystem::path File_int=Append_to_name(File_name,"int");
    boost::filesystem::path File_life=Append_to_name(File_name,"life");
    boost::filesystem::path File_g2_far=Append_to_name(File_name,"g2_far");
    boost::filesystem::path File_g2_norm=Append_to_name(File_name,"g2_norm");
    File_out.replace_extension(".dat");
    File_int.replace_extension(".dat");
    File_life.replace_extension(".dat");
    File_g2_far.replace_extension(".dat");
    File_g2_norm.replace_extension(".dat");

//    if(!boost::filesystem::exists(File_out)){
//        goto end;
//    }
    {
        curveg2->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_g2_far.c_str()));
            std::vector<std::vector<double>> data_toplot;

            //        QwtPlotCurve *curveg2 = new QwtPlotCurve("");

            curveg2->setPen(Qt::blue,4);
            for (unsigned int i = 0; i < datas.size(); ++i) {
                std::vector<double> vec;
                vec.push_back(datas[1][i]);
                vec.push_back(datas[3][i]);
                data_toplot.push_back(vec);
            }
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            curveg2->setSamples(datax,datay);
            curveg2->attach(ui->qwtPlot_g2far);
            ui->qwtPlot_g2far->setAxisTitle(QwtPlot::Axis::xBottom,"time (s)");
            ui->qwtPlot_g2far->setAxisTitle(QwtPlot::Axis::yLeft,"counts");
            ui->qwtPlot_g2far->replot();
        } catch (const char* msg) {
            QMessageBox msgBox;
            msgBox.setText("An exception occurred:");
            msgBox.setInformativeText(msg);
            msgBox.exec();
            goto end;
        }
    }
    //        curveg2=curve1;
end:
    nullptr;
}



boost::filesystem::path MainWindow::Append_to_name(boost::filesystem::path P, std::string string)
{
    std::string rndString = string;
    boost::filesystem::path newPath = P.parent_path() / boost::filesystem::path(P.stem().string() + "_" + rndString + P.extension().string());
    return newPath;
}
