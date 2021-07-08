/****************************************************************************

    xflr5v6
    Copyright (C) André Deperrois
    All rights reserved.

*****************************************************************************/



#include <QWidget>
#include <QDialog>
#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSurface>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QMessageBox>
#include <QSettings>

class PlainTextOutput;
class gl3dTestGLView;
class IntEdit;

class OpenGlDlg : public QDialog
{
    Q_OBJECT
    public:
        OpenGlDlg(QWidget *pParent=nullptr);

        void initDialog();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onCreateContext();
        void onRenderWindowReady();
        void onRenderWindowError(const QString &msg);
        void onSettingsChanged();
        void onMultiSampling();
        void onButton(QAbstractButton *pButton);
        void onApply();
        void reject() override;


    private:
        QSize sizeHint() const override {return QSize(1200,1500);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        void addVersions(QLayout *pLayout);
        void readFormat(QSurfaceFormat &fmt);
        void readVersion(QPair<int, int> &oglversion);

        void printFormat(const QSurfaceFormat &format, QString &log, bool bFull=true);
        void setupLayout();

        void enableControls(QPair<int, int> oglversion);
        QMessageBox::StandardButton applyChanges();

//-----------Variables ----------------

        QPushButton *m_ppbApply;

        QDialogButtonBox *m_pButtonBox;
        QComboBox *m_pcbVersion;
        QCheckBox *m_pchDeprecatedFcts;
        QCheckBox *m_pchMultiSampling;
        PlainTextOutput *m_pptglOutput;

        QLabel *m_plabMemStatus;

        IntEdit *m_pieSamples;

        QRadioButton *m_prbProfiles[3];

        QStackedWidget *m_pStackWt;

        gl3dTestGLView *m_pgl3dTestView;

        QSplitter *m_pHSplitter;

        bool m_bChanged;

        QSurfaceFormat m_SavedFormat;

        static int s_iView;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;
};
