#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

enum EDMWstate { unloaded, loadedUnpatched, loadedPatched };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void patchFileClicked();
    void openFileClicked();
    void saveClicked();
    void saveDefaultClicked();

private:
    void printInfoMessage(QString msg, QString detailMsg = nullptr);
    void printErrorMessage(QString msg, QString detailMsg = nullptr);
    void updateByState();
    void overwriteDataArray(float zoom, float cull);
    void writeFile();
    QLabel *fileLabel;
    QPushButton *patchBtn;
    QLineEdit *zoomEdit;
    QLineEdit *cullEdit;
    QPushButton *saveBtn;
    QPushButton *saveDefaultBtn;

    EDMWstate state;
    QString path;
    std::vector<uint8_t> data;

    uint32_t EDMWUnpatchedSize = 0x479000;
    uint32_t EDMWPatchedSize = 0x479000 + 0xFFF;

    // header unpatched / patched values
    uint32_t EDMWImageSizePos = 0x180;
    uint32_t EDMWUnpatchedImageSize = 0x5dc000;
    uint32_t EDMWPatchedImageSize = 0x5dc000 + 0xFFF;

    uint32_t EDMWSegmentSizePos = 0x136;
    uint16_t EDMWUnpatchedSegmentSize = 5;
    uint16_t EDMWPatchedSegmentSize = 6;

    uint32_t EDMWSegmentNamePos = 0x2F0;
    uint8_t EDMWUnpatchedSegmentName[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t EDMWPatchedSegmentName[8] = { '.','t','e','x','t','2', 0, 0 };

    uint32_t EDMWVirtualSizePos = 0x2F8;
    uint32_t EDMWUnpatchedVirtualSize = 0;
    uint32_t EDMWPatchedVirtualSize = 0xFFF;

    uint32_t EDMWVirtualAddressPos = 0x2FC;
    uint32_t EDMWUnpatchedVirtualAddress = 0x0;
    uint32_t EDMWPatchedVirtualAddress = 0x005DC000;

    uint32_t EDMWSizeOfRawDataPos = 0x300;
    uint32_t EDMWUnpatchedSizeOfRawData = 0x0;
    uint32_t EDMWPatchedSizeOfRawData = 0xFFF;

    uint32_t EDMWPointerToRawDataPos = 0x304;
    uint32_t EDMWUnpatchedPointerToRawData = 0x0;
    uint32_t EDMWPatchedPointerToRawData = 0x479000;

    uint32_t EDMWCharacteristicsPos = 0x314;
    uint8_t EDMWUnpatchedCharacteristics[4] = { 0x0, 0x0, 0x0, 0x0 };
    // Mark section to be readable & executable
    uint8_t EDMWPatchedCharacteristics[4] = { 0xE0, 0x00, 0x00, 0x60 };

    // manipulated render code section
    uint32_t EDMWRedirectSectionPos = 0x17D2CA;
    uint8_t EDMWUnpatchedRedirectSection[6] = { 
        0xD8, 0xCB, // fmul st,st(3)
        0xD8, 0xC1, // fadd st,st(1)
        0xDD, 0xDB  // fstp st(3)
    };
    uint8_t EDMWPatchedRedirectSection[6] = { 
        0xE9, 0x31, 0xED, 0x45, 0x00, // jmp 0x45ed36
        0x90                          // nop
    };

    // additional code section
    uint32_t EDMWPatchedZoomCodeSectionPos = 0x479000;
    uint8_t EDMWPatchedZoomCodeSection[0x19] = {
        0xD8, 0xCB,                   // fmul   st,st(3)
        0x50,                         // push   eax
        0xB8, 0x00, 0x00, 0x80, 0x3F, // mov    eax,0x3f800000  <- default zoom value (1.0f)
        0x50,                         // push   eax
        0xD9, 0x04, 0x24,             // fld    DWORD PTR [esp]
        0xDE, 0xC9,                   // fmulp  st(1),st
        0x58,                         // pop    eax
        0x58,                         // pop    eax
        0xD8, 0xC1,                   // fadd   st,st(1)
        0xDD, 0xDB,                   // fstp   st(3)
        0xE9, 0xB7, 0x12, 0xBA, 0xFF  // jmp    0xffba12d0
    };

    // default patched values
    uint32_t EDMWPatchedZoomScalingPos = 0x479004;
    uint32_t EDMWCullPos = 0x3EE8A4;

    float patchedDefaultZoomScaling = 1.0;
    float patchedDefaultCull = 26.29;
};

#endif // MAINWINDOW_H
