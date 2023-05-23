#include "gnssdialog.h"
#include "ui_gnssdialog.h"

GnssDialog::GnssDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::GnssDialog),
    m_sigStrGraf(new QChartView)
{
    m_ui->setupUi(this);

    //set view on mdiarea    
    m_ui->horizontalLayout_2->addWidget(m_sigStrGraf);

}

GnssDialog::~GnssDialog()
{
    delete m_ui;
}

void GnssDialog::delay(int n)
{
    QEventLoop loop;
    QTimer::singleShot(n, &loop,SLOT(quit()));
    loop.exec();
}

void GnssDialog::putData(const QByteArray &data)//get data from serial
{
    qDebug() << "gnssdialog putData" << data;

    //insertPlainText(data);
    //QScrollBar *bar = verticalScrollBar();
    //bar->setValue(bar->maximum());

    textData = textData + data;
}

void GnssDialog::vaitForOk()
{
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
}

QString GnssDialog::getAnswer(const QString &data)
{
    textData = "";//erase console output buffer
    emit getData(data.toLocal8Bit());//send data to port
    for(int i=0; i<10; i++){
        delay(500);
        if(textData.indexOf("OK")>0) break;
    }//wait for response
    QString infoText = QString::fromStdString(textData.toStdString());
    //qDebug() << "GnssDialog get data0" << infoText;
    //infoText.resize(infoText.size()-8);// cut "OK" from answer
    //infoText=infoText.sliced(2);
    //qDebug() << "GnssDialog get data1" << infoText;
    return infoText;
}

QList<QStringList> GnssDialog::parseGSV(const QString &data)
{
    QStringList NmeaGSV = data.split(',');
    QList<QStringList> result;
    QStringList numSat;
    QStringList satNum;
    QStringList elevation;
    QStringList azimuth;
    QStringList signalStrength;

    //qDebug() << "GnssDialog get NmeaGSV" << NmeaGSV;
    numSat.append(NmeaGSV[2]);
    //qDebug() << "GnssDialog get numGSV" << NmeaGSV.size();

    if(NmeaGSV[1].toInt()<NmeaGSV[0].toInt())//full 4*x satellite message
    {
        satNum.append(NmeaGSV[3]);
        elevation.append(NmeaGSV[4]);
        azimuth.append(NmeaGSV[5]);
        signalStrength.append(NmeaGSV[6]);
        satNum.append(NmeaGSV[7]);
        elevation.append(NmeaGSV[8]);
        azimuth.append(NmeaGSV[9]);
        signalStrength.append(NmeaGSV[10]);
        satNum.append(NmeaGSV[11]);
        elevation.append(NmeaGSV[12]);
        azimuth.append(NmeaGSV[13]);
        signalStrength.append(NmeaGSV[14]);
        satNum.append(NmeaGSV[15]);
        elevation.append(NmeaGSV[16]);
        azimuth.append(NmeaGSV[17]);
        signalStrength.append(NmeaGSV[18]);

    } else //remainder of division by 4 satellite message
    {
        int tail = NmeaGSV[2].toInt() - (NmeaGSV[1].toInt()-1)*4;
        if(tail>=1)
        {
            satNum.append(NmeaGSV[3]);
            elevation.append(NmeaGSV[4]);
            azimuth.append(NmeaGSV[5]);
            signalStrength.append(NmeaGSV[6]);
        }
        if(tail>=2)
        {
            satNum.append(NmeaGSV[7]);
            elevation.append(NmeaGSV[8]);
            azimuth.append(NmeaGSV[9]);
            signalStrength.append(NmeaGSV[10]);
        }
        if(tail>=3)
        {
            satNum.append(NmeaGSV[11]);
            elevation.append(NmeaGSV[12]);
            azimuth.append(NmeaGSV[13]);
            signalStrength.append(NmeaGSV[14]);
        }
        if(tail>=4)
        {
            satNum.append(NmeaGSV[15]);
            elevation.append(NmeaGSV[16]);
            azimuth.append(NmeaGSV[17]);
            signalStrength.append(NmeaGSV[18]);
        }
    }

    result.append(numSat);
    result.append(satNum);
    result.append(elevation);
    result.append(azimuth);
    result.append(signalStrength);
    return result;
}

void GnssDialog::on_refreshPushButton_clicked()
{
    m_ui->refreshPushButton->setEnabled(false);
    //m_ui->altLabel->clear();
    //m_ui->lonLabel->clear();
    //m_ui->latLabel->clear();
    //m_ui->dateLabel->clear();
    //m_ui->isPosLabel->clear();
    //m_ui->utsTimeLabel->clear();

    QString infoText = getAnswer("AT+ZGRUN?\r\n");
    int run = 0;
    if(infoText.indexOf("ZGRUN")>0)
    {
        run = infoText.sliced(infoText.indexOf(":")+2,1).toInt();
        //qDebug() << "GnssDialog get run" << run;
        if (run==0)
        {
            m_ui->StopRadioButton->toggle();
        }
        else if (run==1)
        {
            m_ui->SingleRadioButton->toggle();
        }
        else if (run==2) m_ui->trackRadioButton->toggle();
    }

    if(run == 0)
    {
        m_ui->refreshPushButton->setEnabled(true);
        return;
    }

    infoText = getAnswer("AT+ISPOS?\r\n");
    bool posFlag = false;
    if(infoText.indexOf("NO")>0)
    {
        m_ui->isPosLabel->setText(tr("Position : Not ready"));
    } else if(infoText.indexOf("YES")>0)
    {
        m_ui->isPosLabel->setText(tr("Position : Ready"));
        posFlag = true;
    }

    infoText = getAnswer("AT+ZGSYSTEM?\r\n");
    int gnssSys = -1;
    if(infoText.indexOf("ZGSYSTEM")>0)
    {
        gnssSys = infoText.sliced(infoText.indexOf(":")+2,1).toInt();
        //qDebug() << "GnssDialog get gnssSys" << gnssSys;
        if (gnssSys==0)
        {
            m_ui->gpsRadioButton->toggle();
        }
        else if (gnssSys==1)
        {
            m_ui->bduRadioButton->toggle();
        }
        else if (gnssSys==2)
        {
            m_ui->gpsBduRadioButton->toggle();
        }
        else if (gnssSys==3)
        {
            m_ui->gpsGloRadioButton->toggle();
        }
        else if (gnssSys==4)
        {
            m_ui->gpsGalRadioButton->toggle();
        }
        else if (gnssSys==5)
        {
            m_ui->gpsGloGalRadioButton->toggle();
        }
        else if (gnssSys==6)
        {
            m_ui->gpsBduGalRadioButton->toggle();
        }
    }

    infoText = getAnswer("AT+ZGNMEAREQ=63\r\n");
    QString GGA ="";
    QString RMC ="";    
    if(infoText.indexOf("GGA")>0)
    {
        GGA = infoText.sliced(infoText.indexOf("GGA")+4);
        GGA = GGA.sliced(0,GGA.indexOf("*"));
        //qDebug() << "GnssDialog get GGA" << GGA;
    }
    if(infoText.indexOf("RMC")>0)
    {
        RMC = infoText.sliced(infoText.indexOf("RMC")+4);
        RMC = RMC.sliced(0,RMC.indexOf("*"));
        //qDebug() << "GnssDialog get RMC" << RMC;
    }
    QStringList NmeaGGA = GGA.split(',');
    NmeaGGA[0] = NmeaGGA[0].insert(2,":");
    NmeaGGA[0] = NmeaGGA[0].insert(5,":");
    NmeaGGA[0] = NmeaGGA[0].sliced(0,NmeaGGA[0].indexOf("."));
    QStringList NmeaRMC = RMC.split(',');
    NmeaRMC[8] = NmeaRMC[8].insert(2,".");
    NmeaRMC[8] = NmeaRMC[8].insert(5,".");
    //qDebug() << "GnssDialog get NmeaGGA" << NmeaGGA[0];
    m_ui->utsTimeLabel->setText(tr("UTC time : ") + NmeaGGA[0]);
    m_ui->latLabel->setText(tr("Lat : ") + NmeaGGA[1] + " " + NmeaGGA[2]);
    m_ui->lonLabel->setText(tr("Lon : ") + NmeaGGA[3] + " " + NmeaGGA[4]);
    m_ui->altLabel->setText(tr("Alt : ") + NmeaGGA[8]);
    m_ui->dateLabel->setText(tr("Date : ") + NmeaRMC[8]);

    if(posFlag)//data for graph
    {
        //get signal strength from GSV
        QString GSV ="";
        QString allGSV =infoText;
        QStringList gpsSignalStrength;
        QStringList gpsElevation;
        QStringList gpsAzimuth;
        QStringList gpsSatNum;
        int numGpsSat=0;

        //qDebug() << "GnssDialog get infoText" <<  infoText;
        if(allGSV.indexOf("GPGSV")>0)
        {
            for(int numGSV=0; numGSV<20; numGSV++)
            if(allGSV.indexOf("GPGSV")>=0)//cut CSV strings from string start one by one
            {
                allGSV= allGSV.sliced(allGSV.indexOf("GPGSV")+6);
                //qDebug() << "GnssDialog get GSV" <<  GSV;
                GSV = allGSV.sliced(0,allGSV.indexOf("*"));
                //qDebug() << "GnssDialog get GPGSV" << GSV;

                QList<QStringList> parsed = parseGSV(GSV);
                //qDebug() << "parsed" << parsed;
                QStringList tempNum = parsed[0];
                numGpsSat = tempNum[0].toInt();
                //qDebug() << "numGpsSat" << numGpsSat;
                gpsSatNum.append(parsed[1]);
                gpsElevation.append(parsed[2]);
                gpsAzimuth.append(parsed[3]);
                gpsSignalStrength.append(parsed[4]);
            }
        }

        //qDebug() << "GnssDialog get gpsSatNum" << gpsSatNum;
        //qDebug() << "GnssDialog get gpsSignalStrength" << gpsSignalStrength;
        if(gpsSatNum.size() != gpsSignalStrength.size()) qDebug() << "GnssDialog. Parsed satnum & satsig do not match";
        if(numGpsSat != gpsSignalStrength.size()) qDebug() << "GnssDialog. Parsed sat = " << gpsSignalStrength.size() << " but sat quantity = " << numGpsSat;

        allGSV =infoText;
        QStringList gloSignalStrength;
        QStringList gloElevation;
        QStringList gloAzimuth;
        QStringList gloSatNum;
        int numGloSat=0;

        if(allGSV.indexOf("GLGSV")>0)
        {
            for(int numGSV=0; numGSV<20; numGSV++)
            if(allGSV.indexOf("GLGSV")>=0)//cut CSV strings from string start one by one
            {
                allGSV= allGSV.sliced(allGSV.indexOf("GLGSV")+6);
                //qDebug() << "GnssDialog get GSV" <<  GSV;
                GSV = allGSV.sliced(0,allGSV.indexOf("*"));
                //qDebug() << "GnssDialog get GPGSV" << GSV;

                QList<QStringList> parsed = parseGSV(GSV);
                //qDebug() << "parsed" << parsed;
                QStringList tempNum = parsed[0];
                numGloSat = tempNum[0].toInt();
                //qDebug() << "numGpsSat" << numGpsSat;
                gloSatNum.append(parsed[1]);
                gloElevation.append(parsed[2]);
                gloAzimuth.append(parsed[3]);
                gloSignalStrength.append(parsed[4]);
            }
        }

        //qDebug() << "GnssDialog get gloSatNum" << gloSatNum;
        //qDebug() << "GnssDialog get gloSignalStrength" << gloSignalStrength;
        if(gloSatNum.size() != gloSignalStrength.size()) qDebug() << "GnssDialog. Parsed satnum & satsig do not match";
        if(numGloSat != gloSignalStrength.size()) qDebug() << "GnssDialog. Parsed sat = " << gloSignalStrength.size() << " but sat quantity = " << numGloSat;

        allGSV =infoText;
        QStringList galSignalStrength;
        QStringList galElevation;
        QStringList galAzimuth;
        QStringList galSatNum;
        int numGalSat=0;

        if(allGSV.indexOf("GAGSV")>0)
        {
            for(int numGSV=0; numGSV<20; numGSV++)
            if(allGSV.indexOf("GAGSV")>=0)//cut CSV strings from string start one by one
            {
                allGSV= allGSV.sliced(allGSV.indexOf("GAGSV")+6);
                //qDebug() << "GnssDialog get GSV" <<  GSV;
                GSV = allGSV.sliced(0,allGSV.indexOf("*"));
                //qDebug() << "GnssDialog get GAGSV" << GSV;

                QList<QStringList> parsed = parseGSV(GSV);
                //qDebug() << "parsed" << parsed;
                QStringList tempNum = parsed[0];
                numGalSat = tempNum[0].toInt();
                //qDebug() << "numGpsSat" << numGpsSat;
                galSatNum.append(parsed[1]);
                galElevation.append(parsed[2]);
                galAzimuth.append(parsed[3]);
                galSignalStrength.append(parsed[4]);
            }
        }

        //qDebug() << "GnssDialog get galSatNum" << galSatNum;
        //qDebug() << "GnssDialog get galSignalStrength" << galSignalStrength;
        if(galSatNum.size() != galSignalStrength.size()) qDebug() << "GnssDialog. Parsed satnum & satsig do not match";
        if(numGalSat != galSignalStrength.size()) qDebug() << "GnssDialog. Parsed sat = " << galSignalStrength.size() << " but sat quantity = " << numGalSat;

        allGSV =infoText;
        QStringList bduSignalStrength;
        QStringList bduElevation;
        QStringList bduAzimuth;
        QStringList bduSatNum;
        int numBduSat=0;

        if(allGSV.indexOf("GBGSV")>0)
        {
            for(int numGSV=0; numGSV<20; numGSV++)
            if(allGSV.indexOf("GBGSV")>=0)//cut CSV strings from string start one by one
            {
                allGSV= allGSV.sliced(allGSV.indexOf("GBGSV")+6);
                //qDebug() << "GnssDialog get GSV" <<  GSV;
                GSV = allGSV.sliced(0,allGSV.indexOf("*"));
                //qDebug() << "GnssDialog get GBGSV" << GSV;

                QList<QStringList> parsed = parseGSV(GSV);
                //qDebug() << "parsed" << parsed;
                QStringList tempNum = parsed[0];
                numBduSat = tempNum[0].toInt();
                //qDebug() << "numGpsSat" << numGpsSat;
                bduSatNum.append(parsed[1]);
                bduElevation.append(parsed[2]);
                bduAzimuth.append(parsed[3]);
                bduSignalStrength.append(parsed[4]);
            }
        }

        //qDebug() << "GnssDialog get bduSatNum" << bduSatNum;
        //qDebug() << "GnssDialog get bduSignalStrength" << bduSignalStrength;
        if(bduSatNum.size() != bduSignalStrength.size()) qDebug() << "GnssDialog. Parsed satnum & satsig do not match";
        if(numBduSat != bduSignalStrength.size()) qDebug() << "GnssDialog. Parsed sat = " << bduSignalStrength.size() << " but sat quantity = " << numGalSat;

        if(allGSV.indexOf("GLGSV")>0 || allGSV.indexOf("GBGSV")>0 || allGSV.indexOf("GAGSV")>0 || allGSV.indexOf("GLGSV")>0)//at least one system data exist
        {
            if(m_ui->gnssGraphTypeComboBox->currentIndex()==0)//draw sattellite signal graph
            {
                // create sets of data -> GNSS system
                QBarSet *gpsSys = new QBarSet("GPS");
                QBarSet *gloSys = new QBarSet("GLONASS");
                QBarSet *galSys = new QBarSet("Galileo");
                QBarSet *bduSys = new QBarSet("BeiDou");
                //QBarSet *qzsSys = new QBarSet("QZSS");
                //QBarSet *sbsSys = new QBarSet("SBAS");
                //QBarSet *irnSys = new QBarSet("IRNSS");

                //add data to sets -> SNR for each sattellite
                if(gpsSignalStrength.size()>0)
                    for(int i=0; i<gpsSignalStrength.size();i++)
                    {
                        int test = gpsSignalStrength[i].toInt();
                        *gpsSys <<test ;
                    }

                if(gloSignalStrength.size()>0)
                    for(int i=0; i<gloSignalStrength.size();i++)
                    {
                        int test = gloSignalStrength[i].toInt();
                        *gloSys <<test ;
                    }

                if(galSignalStrength.size()>0)
                    for(int i=0; i<galSignalStrength.size();i++)
                    {
                        int test = galSignalStrength[i].toInt();
                        *galSys <<test ;
                    }

                if(bduSignalStrength.size()>0)
                    for(int i=0; i<bduSignalStrength.size();i++)
                    {
                        int test = bduSignalStrength[i].toInt();
                        *bduSys <<test ;
                    }

                //*qzsSys << 2291.97 << 2312.6 << 2222.4 << 1992.1;
                //*sbsSys << 921.92  << 891.5  << 862.1  << 834.8;
                //*irnSys << 1349.41 << 1362.9 << 1313.8 << 1139.8;

                //create series for each set of data
                QBarSeries *series = new QBarSeries();
                series->append(gpsSys);
                series->append(gloSys);
                series->append(galSys);
                series->append(bduSys);

                //series->append(qzsSys);
                //series->append(sbsSys);
                //series->append(irnSys);

                //create main chart object
                QChart *chart = new QChart();
                chart->addSeries(series);
                chart->setTitle(tr("GNNS signal stength"));
                chart->setAnimationOptions(QChart::SeriesAnimations);

                //create & set categories -> gpsSatNum
                //QStringList categories;
                //categories << "GPS 17" << "GLO 01" << "GAL 19" << "BDU 09";

                //create & set axis
                QBarCategoryAxis *axisX = new QBarCategoryAxis();
                axisX->append(gpsSatNum);
                chart->addAxis(axisX, Qt::AlignBottom);
                series->attachAxis(axisX);

                QValueAxis *axisY = new QValueAxis();
                axisY->setRange(0,100);
                chart->addAxis(axisY, Qt::AlignLeft);
                series->attachAxis(axisY);

                //add legend
                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignRight);

                //add chart to view
                m_sigStrGraf->setChart(chart);
                m_sigStrGraf->setRenderHint(QPainter::Antialiasing);

                //set view on mdiarea
                //m_ui->mdiArea->addSubWindow(chartView);
                //m_sigStrGraf=chartView;
                //m_sigStrGraf->showMaximized();
            }
            if(m_ui->gnssGraphTypeComboBox->currentIndex()==1)//draw sattellite polar plot
            {
                //create sets of data -> GNSS system
                QScatterSeries *gpsSys = new QScatterSeries();
                gpsSys->setName("GPS");
                gpsSys->setMarkerShape(QScatterSeries::MarkerShapeCircle);
                gpsSys->setMarkerSize(10.0);
                QScatterSeries *gloSys = new QScatterSeries();
                gloSys->setName("GLONASS");
                gloSys->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
                gloSys->setMarkerSize(10.0);
                QScatterSeries *galSys = new QScatterSeries();
                galSys->setName("Galileo");
                galSys->setMarkerShape(QScatterSeries::MarkerShapeRotatedRectangle);
                galSys->setMarkerSize(10.0);
                QScatterSeries *bduSys = new QScatterSeries();
                bduSys->setName("BeiDou");
                bduSys->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
                bduSys->setMarkerSize(10.0);

                //QScatterSeries *qzsSys = new QScatterSeries("QZSS");
                //QScatterSeries *sbsSys = new QScatterSeries("SBAS");
                //QScatterSeries *irnSys = new QScatterSeries("IRNSS");

                qDebug()<<"bduElevation"<<bduElevation;
                qDebug()<<"bduAzimuth"<<bduAzimuth;
                qDebug()<<"numBduSat"<<numBduSat;
                for(int i=0; i<numGpsSat; i++) if(gpsAzimuth[i]!="" && gpsElevation[i]!="")gpsSys->append(gpsAzimuth[i].toInt(), 90-gpsElevation[i].toInt());
                for(int i=0; i<numGloSat; i++) if(gloAzimuth[i]!="" && gloElevation[i]!="")gloSys->append(gloAzimuth[i].toInt(), 90-gloElevation[i].toInt());
                for(int i=0; i<numGalSat; i++) if(galAzimuth[i]!="" && galElevation[i]!="")galSys->append(galAzimuth[i].toInt(), 90-galElevation[i].toInt());
                for(int i=0; i<numBduSat; i++) if(bduAzimuth[i]!="" && bduElevation[i]!="")bduSys->append(bduAzimuth[i].toInt(), 90-bduElevation[i].toInt());

                //create main chart object
                QPolarChart *chart = new QPolarChart();
                chart->setTitle(tr("Polar plot"));

                chart->addSeries(gpsSys);
                chart->addSeries(gloSys);
                chart->addSeries(galSys);
                chart->addSeries(bduSys);

                QValueAxis *angularAxis = new QValueAxis();
                angularAxis->setTickCount(13); // First and last ticks are co-located on 0/360 angle.
                angularAxis->setLabelFormat("%d");
                angularAxis->setShadesVisible(true);
                angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
                chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
                angularAxis->setRange(0, 360);

                QCategoryAxis *radialAxis = new QCategoryAxis;
                radialAxis->setRange(0, 90);
                for(unsigned int i = 0; i <= 90; i += 15)
                    radialAxis->append(QString::number(90-i), i);
                radialAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
                radialAxis->setLabelsVisible(true);
                chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

                //add chart to view
                m_sigStrGraf->setChart(chart);
                m_sigStrGraf->setRenderHint(QPainter::Antialiasing);
            }
        }
    }


    m_ui->refreshPushButton->setEnabled(true);
}

void GnssDialog::on_StopRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGRUN=0\r\n");//send data to port
    vaitForOk();
}

void GnssDialog::on_SingleRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGRUN=1\r\n");//send data to port
    vaitForOk();
}

void GnssDialog::on_trackRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGRUN=2\r\n");//send data to port
    vaitForOk();
}

void GnssDialog::on_gpsRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=0\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_bduRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=1\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gpsBduRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=2\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gpsGloRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=3\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gpsGalRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=4\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gpsGloGalRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=5\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gpsBduGalRadioButton_clicked(bool checked)
{
    emit getData("AT+ZGSYSTEM=6\r\n");//send data to port
    vaitForOk();
}


void GnssDialog::on_gnssResetPushButton_clicked()
{
    getAnswer("AT+ZGRST="+QString::number(m_ui->gnssResetTypeComboBox->currentIndex())+"\r\n");
}

