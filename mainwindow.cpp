#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "measure.h"

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
    ui->lineEdit_g2norm_x0->setValidator(doublevalidator);
    curveg2 = new QwtPlotCurve("");
    curveg2_n = new QwtPlotCurve("");
    curveg2far = new QwtPlotCurve("");
    curvelifetime = new QwtPlotCurve("");
    curveTimeTrace= new QwtPlotCurve("");
    curveHistFreq= new QwtPlotCurve("");
    grid = new QwtPlotGrid();
    //    curveHistFreq=  new QwtPlotHistogram("");
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
    ui->pushButton_Analize->setDisabled(true);
    ui->progressBar_mesure->reset();
    ui->progressBar_mesure->setRange(0,0);
    ui->progressBar_mesure->setValue(0);
    QString text=ui->pushButton_Analize->text();
    ui->pushButton_Analize->setText("Analisis in progress");
    int MC;
    bool flag_normalization=false;
    int sogliaGlob=0;
    int g2width=10;
    double altAt=0;
    double intTime=0.01;
    int binNum=50;

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

    if(ui->lineEdit_BinForPulse->text().toInt()>0){
        binNum=ui->lineEdit_BinForPulse->text().toInt();
    }else{
        QMessageBox msgBox;
        msgBox.setText("Number of bins must bigger than 0");
        msgBox.exec();
        goto end;
    }

    {
        ui->textEdit_terminal->append(ui->textEdit_fileName->toPlainText());
//        Measure *M= new Measure(ui->textEdit_fileName->toPlainText().toStdString(), MC, flag_normalization, sogliaGlob, binNum, g2width, altAt, intTime);
        Measure *M= new Measure(ui->textEdit_fileName->toPlainText().toStdString(), MC, flag_normalization, sogliaGlob, binNum, g2width, altAt, intTime, 0, ui->textEdit_terminal, ui->progressBar_mesure);

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
    set_file_names();

    {
        curveg2->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_out));

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


void MainWindow::on_pushButton_g2far_clicked(){
    set_file_names();
    ui->qwtPlot_g2far->setTitle("g2 plot");
    ui->qwtPlot_g2far->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));

    {
        curveg2far->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_g2_far));
            std::vector<std::vector<double>> data_toplot;

            curveg2far->setPen(Qt::blue,4);

            for (unsigned int i = 0; i < datas[0].size(); ++i) {
                std::vector<double> vec;
                if(datas[1][i]>1e-6){
                vec.push_back(datas[1][i]);
                vec.push_back(datas[3][i]);
                data_toplot.push_back(vec);
                }
            }
            data_toplot=trasposeTable(data_toplot);
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            curveg2far->setSamples(datax,datay);
            curveg2far->attach(ui->qwtPlot_g2far);
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


void MainWindow::on_pushButton_plot_lifetime_clicked(){
    ui->qwtPlot_lifetime->setTitle("g2 plot");
    set_file_names();
    if(ui->checkBox_lifetimeLogY->isChecked()) {
        ui->qwtPlot_lifetime->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
    }else{
        ui->qwtPlot_lifetime->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
    }
    std::string FileName = ui->textEdit_fileName->toPlainText().toStdString();
    {
        curvelifetime->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_life));
            std::vector<std::vector<double>> data_toplot;

            //        QwtPlotCurve *curveg2 = new QwtPlotCurve("");

            curvelifetime->setPen(Qt::blue,4);

            for (unsigned int i = 0; i < datas[0].size(); ++i) {
                std::vector<double> vec;
                if(datas[2][i]>0){ //removes zero element at the end
                    vec.push_back(datas[1][i]);
                    vec.push_back(datas[2][i]);
                    data_toplot.push_back(vec);
                }

            }
            data_toplot.pop_back(); //removes the last element that normally has some problem
            data_toplot=trasposeTable(data_toplot);
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            curvelifetime->setSamples(datax,datay);
            curvelifetime->attach(ui->qwtPlot_lifetime);
            ui->qwtPlot_lifetime->setAxisTitle(QwtPlot::Axis::xBottom,"time (s)");
            ui->qwtPlot_lifetime->setAxisTitle(QwtPlot::Axis::yLeft,"counts");
            ui->qwtPlot_lifetime->replot();
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


void MainWindow::on_pushButton_plot_TimeTrace_clicked(){
    ui->qwtPlot_TimeTrace->setTitle("Time trace plot");
    set_file_names();
    std::string FileName = ui->textEdit_fileName->toPlainText().toStdString();

    {
        curveTimeTrace->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_int));
            std::vector<std::vector<double>> data_toplot;

            //        QwtPlotCurve *curveg2 = new QwtPlotCurve("");

            curveTimeTrace->setPen(Qt::blue,4);

            for (unsigned int i = 0; i < datas[0].size(); ++i) {
                std::vector<double> vec;
                if(datas[1][i]>0){ //removes zero element at the end
                    vec.push_back(datas[0][i]);
                    vec.push_back(datas[1][i]);
                    data_toplot.push_back(vec);
                }

            }
            data_toplot.pop_back(); //removes the last element that normally has some problem
            data_toplot=trasposeTable(data_toplot);
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            curveTimeTrace->setSamples(datax,datay);
            curveTimeTrace->attach(ui->qwtPlot_TimeTrace);
            ui->qwtPlot_TimeTrace->setAxisTitle(QwtPlot::Axis::xBottom,"time (s)");
            ui->qwtPlot_TimeTrace->setAxisTitle(QwtPlot::Axis::yLeft,"counts");
            ui->qwtPlot_TimeTrace->replot();
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

void MainWindow::on_pushButton_plot_FreqHist_clicked(){
    ui->qwtPlot_FreqHist->setTitle("Frequency Histogram plot");
    set_file_names();
    {
        curveHistFreq->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_int));
            std::vector<std::vector<double>> data_toplot;

            //        QwtPlotCurve *curveg2 = new QwtPlotCurve("");

            curveHistFreq->setPen(Qt::blue,4);

            for (unsigned int i = 0; i < datas[0].size(); ++i) {
                std::vector<double> vec;
                if(datas[1][i]>0){ //removes zero element at the end
                    vec.push_back(datas[0][i]);
                    vec.push_back(datas[1][i]);
                    data_toplot.push_back(vec);
                }

            }
            data_toplot.pop_back(); //removes the last element that normally has some problem
            data_toplot=trasposeTable(data_toplot);
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            //this time I whant to create an histogram
            double ymax=0;
            for (double val : datay) {
                if(val>ymax) ymax=val;
            }
            QVector<double> histogram = *new QVector<double>(int(ymax),0);
            QVector<double> counts = *new QVector<double>(int(ymax),0);

            for(double val : datay){
                int valint = int(val);
                if(valint>0) histogram[valint-1]+=1;
            }
            for (int i = 0; i < counts.size(); ++i) {
                counts[i] =double(i);
            }

            curveHistFreq->setSamples(counts,histogram);
//            curveHistFreq->setSamples(datay);
            curveHistFreq->attach(ui->qwtPlot_FreqHist);
            ui->qwtPlot_FreqHist->setAxisTitle(QwtPlot::Axis::xBottom,"counts");
            ui->qwtPlot_FreqHist->setAxisTitle(QwtPlot::Axis::yLeft,"frequencies");
            ui->qwtPlot_FreqHist->replot();
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


void MainWindow::on_pushButton_graphg2_n_clicked()
{
    ui->qwtPlot_g2_n->setTitle("Normalized g2 plot");
    double x0=ui->lineEdit_g2norm_x0->text().toDouble();
    double interval=1e-7;
    double g2_0=0, min_curve=1;
    set_file_names();
    {
        curveg2_n->detach();
        try{
            std::vector< std::vector<double> > datas = trasposeTable(readIn2dData(File_g2_norm));
            std::vector<std::vector<double>> data_toplot;
            std::vector<std::vector<double>> data_center;

            curveg2_n->setPen(Qt::blue,4);

            for (unsigned int i = 0; i < datas[0].size(); ++i) {
                std::vector<double> vec;
                if(1){
                    vec.push_back(datas[1][i]-x0);
                    vec.push_back(datas[2][i]);
                    data_toplot.push_back(vec);
                    if(abs(datas[1][i]-x0)<interval){
                        data_center.push_back(vec);
                        g2_0=g2_0>vec[1]?g2_0:vec[1];
                        min_curve=min_curve<vec[1]?min_curve:vec[1];
                    }
                }
            }
            ui->lineEdit_min->setText(QString::number(min_curve));
            ui->lineEdit_g20->setText(QString::number(g2_0));
            double g2_clean;
            g2_clean=g2_0+min_curve-2*sqrt(g2_0)*sqrt(min_curve);
            g2_clean=g2_clean/(1+min_curve-2*sqrt(min_curve));
            ui->lineEdit_g20_clean->setText(QString::number(g2_clean));
            QString clip;
            clip= QString::number(g2_0)+'\t'+QString::number(g2_clean);
            QClipboard *clipboard= QApplication::clipboard();
            clipboard->setText(clip);
            data_toplot=trasposeTable(data_toplot);
            data_center=trasposeTable(data_center);
            QVector<double> datax= QVector<double>::fromStdVector(data_toplot[0]);
            QVector<double> datay= QVector<double>::fromStdVector(data_toplot[1]);
            curveg2_n->setSamples(datax,datay);
            curveg2_n->attach(ui->qwtPlot_g2_n);
            ui->qwtPlot_g2_n->setAxisTitle(QwtPlot::Axis::xBottom,"time (s)");
            ui->qwtPlot_g2_n->setAxisTitle(QwtPlot::Axis::yLeft,"g2");
            grid->setMajorPen(QPen(Qt::DotLine));
            grid->attach(ui->qwtPlot_g2_n);
            ui->qwtPlot_g2_n->replot();
            QFileInfo File_Name= *new QFileInfo(ui->textEdit_fileName->toPlainText());
            pdf_g2_norm=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_g2_norm.pdf";
            ui->pushButton_g2nSave->setEnabled(true);
        } catch (const char* msg) {
            QMessageBox msgBox;
            msgBox.setText("An exception occurred:");
            msgBox.setInformativeText(msg);
            msgBox.exec();
        }
    }
}

void MainWindow::on_pushButton_g2nSave_clicked()
{
    QwtPlotRenderer *render=new QwtPlotRenderer();
    render->renderDocument(ui->qwtPlot_g2_n,pdf_g2_norm.filePath(),"pdf",QSizeF(100,80));
}

void MainWindow::set_file_names(){
    QFileInfo File_Name= *new QFileInfo(ui->textEdit_fileName->toPlainText());
//    QFileInfo File_Name= *new QFileInfo(QString::fromStdString(FileName));
    File_out=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_out.dat";
    File_int=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_int.dat";
    File_life=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_life.dat";
    File_g2_far=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_g2_far.dat";
    File_g2_norm=File_Name.canonicalPath()+QDir::separator()+File_Name.baseName()+"_g2_norm.dat";

}


