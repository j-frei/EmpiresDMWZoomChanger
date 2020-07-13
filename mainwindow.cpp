#include "mainwindow.h"
#include <QtWidgets>
#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>

MainWindow::MainWindow()

{
    MainWindow::state = EDMWstate::unloaded;

    // Set layout in QWidget
    QWidget *window = new QWidget();

    // Set layout
    QGridLayout *layout = new QGridLayout(window);

    QLabel *infolabel = new QLabel;
    infolabel->setText("Select the file to patch (Empires_DMW.exe)");
    layout->addWidget(infolabel,0,0,1,2);

    QPushButton *loadBtn = new QPushButton;
    loadBtn->setText("open File");
    layout->addWidget(loadBtn,1,0,1,2);

    fileLabel = new QLabel;
    fileLabel->setText("No file loaded");
    layout->addWidget(fileLabel,2,0,1,2);

    patchBtn = new QPushButton;
    patchBtn->setText("patch loaded File");
    layout->addWidget(patchBtn,3,0,1,2);

    // Zoom Input
    QLabel *zoomEditInfo = new QLabel;
    zoomEditInfo->setText("Zoom Scaling (default: 1.0)");
    layout->addWidget(zoomEditInfo,4,0,1,1);

    zoomEdit = new QLineEdit;
    //zoomEdit->setText(QString::fromStdString(std::to_string(-20.5)));
    zoomEdit->setText(QString::number(-20.5));
    layout->addWidget(zoomEdit,4,1,1,1);

    // Cull/Fog Input
    QLabel *cullEditInfo = new QLabel;
    cullEditInfo->setText("Cull, (default: 26.29)");
    layout->addWidget(cullEditInfo,5,0,1,1);

    cullEdit = new QLineEdit;
    //cullEdit->setText(QString::fromStdString(std::to_string(35.0)));
    cullEdit->setText(QString::number(35.0));
    layout->addWidget(cullEdit,5,1,1,1);

    // Save Button
    saveBtn = new QPushButton;
    saveBtn->setText("Write values");
    layout->addWidget(saveBtn,6,0,1,2);

    saveDefaultBtn = new QPushButton;
    saveDefaultBtn->setText("Write default values");
    layout->addWidget(saveDefaultBtn,7,0,1,2);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint );
    setUnifiedTitleAndToolBarOnMac(true);

    connect( patchBtn, SIGNAL(clicked()),this,SLOT(patchFileClicked()));
    connect( loadBtn, SIGNAL(clicked()),this,SLOT(openFileClicked()));
    connect( saveBtn, SIGNAL(clicked()),this,SLOT(saveClicked()));
    connect( saveDefaultBtn, SIGNAL(clicked()),this,SLOT(saveDefaultClicked()));
    updateByState();

}


void MainWindow::updateByState(){
    QString stateStringEDMW;
    float zoomValue = patchedDefaultZoomScaling;
    float cullValue = patchedDefaultCull;

    switch (state) {
    case unloaded:
        patchBtn->setEnabled(false);
        patchBtn->setText("patch loaded File");

        zoomEdit->setEnabled(false);
        cullEdit->setEnabled(false);
        saveBtn->setEnabled(false);
        saveDefaultBtn->setEnabled(false);

        fileLabel->setText("No file loaded");

        zoomEdit->setText(QString::number(zoomValue));
        cullEdit->setText(QString::number(cullValue));

        break;
    case loadedUnpatched:
        patchBtn->setEnabled(true);
        patchBtn->setText("patch loaded File");

        zoomEdit->setEnabled(false);
        cullEdit->setEnabled(false);
        saveBtn->setEnabled(true);
        saveDefaultBtn->setEnabled(false);

        stateStringEDMW = QString("File loaded (unpatched):\n");
        stateStringEDMW.append(path);
        fileLabel->setText(stateStringEDMW);

        zoomValue = patchedDefaultZoomScaling;
        cullValue = patchedDefaultCull;

        zoomEdit->setText(QString::number(zoomValue));
        cullEdit->setText(QString::number(cullValue));
        break;
    case loadedPatched:
        patchBtn->setEnabled(true);
        patchBtn->setText("unpatch loaded File");

        zoomEdit->setEnabled(true);
        cullEdit->setEnabled(true);
        saveBtn->setEnabled(true);
        saveDefaultBtn->setEnabled(true);

        stateStringEDMW = QString("File loaded (patched):\n");
        stateStringEDMW.append(path);
        fileLabel->setText(stateStringEDMW);

        zoomValue = *(reinterpret_cast<float*>(data.data()+EDMWPatchedZoomScalingPos));
        cullValue = *(reinterpret_cast<float*>(data.data()+EDMWCullPos));

        zoomEdit->setText(QString::number(zoomValue));
        cullEdit->setText(QString::number(cullValue));

        break;
    default:
        std::cerr << "Unknown state" << std::endl;
        break;
    }
}

void MainWindow::patchFileClicked(){
    uint32_t *imgSize = nullptr;
    uint16_t *segSize = nullptr;
    uint8_t *segName = nullptr;
    uint32_t *segVirtualSize = nullptr;
    uint32_t *segVirtualAddress = nullptr;
    uint32_t *segSizeOfRawData = nullptr;
    uint32_t *segPointerToRawData = nullptr;
    uint8_t *segCharacteristics = nullptr;
    uint8_t *redirectSection = nullptr;
    uint8_t *zoomCodeSection = nullptr;
    float *dataZoomScaling = nullptr;
    float *dataCull = nullptr;

    // prepare data vector
    if (state == loadedUnpatched){
        // Increase / resize data vector
        data.insert(data.end(), EDMWPatchedSize - EDMWUnpatchedSize, 0x0);
    }
    if (state == loadedPatched){        
        // Shrink / resize data vector
        data.erase(data.begin()+EDMWUnpatchedSize, data.end());
    }

    // assign shared data pointers
    imgSize = reinterpret_cast<uint32_t*>(data.data()+EDMWImageSizePos);
    segSize = reinterpret_cast<uint16_t*>(data.data()+EDMWSegmentSizePos);
    segName = reinterpret_cast<uint8_t*>(data.data()+EDMWSegmentNamePos);
    segVirtualSize = reinterpret_cast<uint32_t*>(data.data()+EDMWVirtualSizePos);
    segVirtualAddress = reinterpret_cast<uint32_t*>(data.data()+EDMWVirtualAddressPos);
    segSizeOfRawData = reinterpret_cast<uint32_t*>(data.data()+EDMWSizeOfRawDataPos);
    segPointerToRawData = reinterpret_cast<uint32_t*>(data.data()+EDMWPointerToRawDataPos);
    segCharacteristics = reinterpret_cast<uint8_t*>(data.data()+EDMWCharacteristicsPos);
    redirectSection = reinterpret_cast<uint8_t*>(data.data()+EDMWRedirectSectionPos);

    // Reset rear cull factor
    dataCull = reinterpret_cast<float*>(data.data()+EDMWCullPos);
    *dataCull = patchedDefaultCull;

    if (state == loadedUnpatched) {
        // assign data pointers for patched setup
        dataZoomScaling = reinterpret_cast<float*>(data.data()+EDMWPatchedZoomScalingPos);
        zoomCodeSection = reinterpret_cast<uint8_t*>(data.data()+EDMWPatchedZoomCodeSectionPos);

        // Patch header
        *imgSize = EDMWPatchedImageSize;
        *segSize = EDMWPatchedSegmentSize;
        std::copy(std::begin(EDMWPatchedSegmentName), std::end(EDMWPatchedSegmentName), segName);
        *segVirtualSize = EDMWPatchedVirtualSize;
        *segVirtualAddress = EDMWPatchedVirtualAddress;
        *segSizeOfRawData = EDMWPatchedSizeOfRawData;
        *segPointerToRawData = EDMWPatchedPointerToRawData;
        std::copy(std::begin(EDMWPatchedCharacteristics), std::end(EDMWPatchedCharacteristics), segCharacteristics);

        // Modify render path
        std::copy(std::begin(EDMWPatchedRedirectSection),std::end(EDMWPatchedRedirectSection), redirectSection);
        // Append additional zoom scaling code
        std::copy(std::begin(EDMWPatchedZoomCodeSection),std::end(EDMWPatchedZoomCodeSection), zoomCodeSection);
        // Set default zoom / cull values
        *dataZoomScaling = patchedDefaultZoomScaling;
        *dataCull = patchedDefaultCull;

        state = loadedPatched;
    } else if (state == loadedPatched){
        // Unpatch header
        *imgSize = EDMWUnpatchedImageSize;;
        *segSize = EDMWUnpatchedSegmentSize;
        std::copy(std::begin(EDMWUnpatchedSegmentName), std::end(EDMWUnpatchedSegmentName), segName);
        *segVirtualSize = EDMWUnpatchedVirtualSize;
        *segVirtualAddress = EDMWUnpatchedVirtualAddress;
        *segSizeOfRawData = EDMWUnpatchedSizeOfRawData;
        *segPointerToRawData = EDMWUnpatchedPointerToRawData;
        std::copy(std::begin(EDMWUnpatchedCharacteristics), std::end(EDMWUnpatchedCharacteristics), segCharacteristics);

        // Restore render path
        std::copy(std::begin(EDMWUnpatchedRedirectSection),std::end(EDMWUnpatchedRedirectSection), redirectSection);
        // Reset rear cull factor
        *dataCull = patchedDefaultCull;

        state = loadedUnpatched;
    }
    updateByState();
}

void MainWindow::openFileClicked() {
    QString filepath = QFileDialog::getOpenFileName();
    std::cout << "Try to open file from: " << filepath.toStdString() << std::endl;

    std::ifstream inputFile (filepath.toStdString(),std::ios::binary);
    if(inputFile.good()){
        // read file
        std::vector<char> const file(
            (std::istreambuf_iterator<char>(inputFile)),
            (std::istreambuf_iterator<char>())
            );
        MainWindow::path = filepath;
        MainWindow::data = std::vector<uint8_t>(file.begin(), file.end());

        if (file.size() == EDMWUnpatchedSize) {
            std::cout << "File is: Empires Dawn of the Modern World (unpatched)" << std::endl;
            MainWindow::state = EDMWstate::loadedUnpatched;
        } else if (file.size() == EDMWPatchedSize) {
            std::cout << "File is: Empires Dawn of the Modern World (patched)" << std::endl;
            MainWindow::state = EDMWstate::loadedPatched;
        } else {
            std::cout << "Unknown file with size " << std::to_string(file.size()) << std::endl;
            printErrorMessage("Unknown file! A file of the following size is expected: 0x479000, 0x479FFF bytes");
            state = EDMWstate::unloaded;
        }
    } else {
        printErrorMessage("Error while reading the file");
        state = EDMWstate::unloaded;
    }
    inputFile.close();
    updateByState();
}

void MainWindow::saveClicked() {
    float zoomValue = patchedDefaultZoomScaling;
    float cullValue = patchedDefaultCull;
    bool validZoom = true;
    bool validCull = true;

    if (state == loadedPatched) {
        // check text fields
        try {
            zoomValue = std::stof(zoomEdit->text().toStdString());
        } catch (...) {
            validZoom = false;
        }
        try {
            cullValue = std::stof(cullEdit->text().toStdString());
        } catch (...) {
            validCull = false;
        }
    }

    if (validZoom && validCull) {
        overwriteDataArray(zoomValue,cullValue);
        // Write data array to file
        writeFile();

    } else {
        QString err = "Invalid values.";
        if (!validZoom)
            err.append("\nZoom value is invalid");
        if (!validCull)
            err.append("\nCull value is invalid");

        printErrorMessage(err);
    }

    updateByState();
}

void MainWindow::saveDefaultClicked(){
    overwriteDataArray(patchedDefaultZoomScaling,patchedDefaultCull);
    // Write data array to file
    writeFile();

    updateByState();
}

void MainWindow::overwriteDataArray(float zoom, float cull){
    float *zoomPtr = nullptr;
    float *cullPtr = nullptr;

    // find float positions in std::vector data
    cullPtr = reinterpret_cast<float*>(data.data()+EDMWCullPos);
    *cullPtr = cull;
    // Write zoom scaling only if data is patched!
    if (state == loadedPatched) {
        zoomPtr = reinterpret_cast<float*>(data.data()+EDMWPatchedZoomScalingPos);
        *zoomPtr = zoom;

    }
}

void MainWindow::writeFile(){
    // try to write file
    std::ofstream outFile (path.toStdString(),std::ios::out | std::ios::binary);
    if (outFile.good()){
        outFile.write(reinterpret_cast<char*>(data.data()),data.size());
        outFile.flush();
        printInfoMessage("File was successfully patched!");
    } else {
        printErrorMessage("Error during file writing.","You can try to run this program as run as administrator.");
    }
    outFile.close();
}

void MainWindow::printErrorMessage(QString msg,QString detailMsg){
    QMessageBox errMsg;
    errMsg.setIcon(QMessageBox::Warning);
    if (detailMsg != nullptr)
        errMsg.setInformativeText(detailMsg);
    errMsg.setText(msg);
    errMsg.exec();
}

void MainWindow::printInfoMessage(QString msg,QString detailMsg){
    QMessageBox infoMsg;
    infoMsg.setIcon(QMessageBox::Information);
    if (detailMsg != nullptr)
        infoMsg.setInformativeText(detailMsg);
    infoMsg.setText(msg);
    infoMsg.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    //event->ignore();
}
