/********************************************************************************
** Form generated from reading UI file 'musicplayer.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MUSICPLAYER_H
#define UI_MUSICPLAYER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_musicplayerClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *fileTable;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *labelTotals;
    QLabel *labelSize;
    QLabel *labelTime;
    QVBoxLayout *verticalLayout_2;
    QPushButton *addFilesButton;
    QPushButton *removeFilesButton;
    QPushButton *selectAllButton;
    QSpacerItem *verticalSpacer;
    QPushButton *upButton;
    QPushButton *downButton;
    QSpacerItem *verticalSpacer_2;
    QFrame *line;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QComboBox *comPortComboBox;
    QPushButton *downloadButton;

    void setupUi(QMainWindow *musicplayerClass)
    {
        if (musicplayerClass->objectName().isEmpty())
            musicplayerClass->setObjectName(QStringLiteral("musicplayerClass"));
        musicplayerClass->resize(797, 446);
        QIcon icon;
        icon.addFile(QStringLiteral("Music-Note-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        musicplayerClass->setWindowIcon(icon);
        centralWidget = new QWidget(musicplayerClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QFont font;
        font.setPointSize(9);
        centralWidget->setFont(font);
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(10);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(10, 10, 10, 10);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        fileTable = new QTableWidget(centralWidget);
        if (fileTable->columnCount() < 4)
            fileTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        fileTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        fileTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        fileTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        fileTable->setObjectName(QStringLiteral("fileTable"));
        fileTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
        fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        fileTable->horizontalHeader()->setStretchLastSection(false);

        verticalLayout_3->addWidget(fileTable);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        labelTotals = new QLabel(centralWidget);
        labelTotals->setObjectName(QStringLiteral("labelTotals"));

        horizontalLayout->addWidget(labelTotals);

        labelSize = new QLabel(centralWidget);
        labelSize->setObjectName(QStringLiteral("labelSize"));

        horizontalLayout->addWidget(labelSize);

        labelTime = new QLabel(centralWidget);
        labelTime->setObjectName(QStringLiteral("labelTime"));

        horizontalLayout->addWidget(labelTime);


        verticalLayout_3->addLayout(horizontalLayout);


        horizontalLayout_4->addLayout(verticalLayout_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        addFilesButton = new QPushButton(centralWidget);
        addFilesButton->setObjectName(QStringLiteral("addFilesButton"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(addFilesButton->sizePolicy().hasHeightForWidth());
        addFilesButton->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(addFilesButton, 0, Qt::AlignTop);

        removeFilesButton = new QPushButton(centralWidget);
        removeFilesButton->setObjectName(QStringLiteral("removeFilesButton"));

        verticalLayout_2->addWidget(removeFilesButton, 0, Qt::AlignTop);

        selectAllButton = new QPushButton(centralWidget);
        selectAllButton->setObjectName(QStringLiteral("selectAllButton"));

        verticalLayout_2->addWidget(selectAllButton);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Preferred);

        verticalLayout_2->addItem(verticalSpacer);

        upButton = new QPushButton(centralWidget);
        upButton->setObjectName(QStringLiteral("upButton"));

        verticalLayout_2->addWidget(upButton);

        downButton = new QPushButton(centralWidget);
        downButton->setObjectName(QStringLiteral("downButton"));

        verticalLayout_2->addWidget(downButton);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);


        horizontalLayout_4->addLayout(verticalLayout_2);


        verticalLayout->addLayout(horizontalLayout_4);

        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_3->addWidget(label_2);

        comPortComboBox = new QComboBox(centralWidget);
        comPortComboBox->setObjectName(QStringLiteral("comPortComboBox"));

        horizontalLayout_3->addWidget(comPortComboBox, 0, Qt::AlignLeft);

        downloadButton = new QPushButton(centralWidget);
        downloadButton->setObjectName(QStringLiteral("downloadButton"));
        downloadButton->setMinimumSize(QSize(0, 32));

        horizontalLayout_3->addWidget(downloadButton);

        horizontalLayout_3->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_3);

        musicplayerClass->setCentralWidget(centralWidget);

        retranslateUi(musicplayerClass);

        QMetaObject::connectSlotsByName(musicplayerClass);
    } // setupUi

    void retranslateUi(QMainWindow *musicplayerClass)
    {
        musicplayerClass->setWindowTitle(QApplication::translate("musicplayerClass", "Music Player - Downloader", 0));
        QTableWidgetItem *___qtablewidgetitem = fileTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("musicplayerClass", "File", 0));
        QTableWidgetItem *___qtablewidgetitem1 = fileTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("musicplayerClass", "Size", 0));
        QTableWidgetItem *___qtablewidgetitem2 = fileTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("musicplayerClass", "Tracks", 0));
        QTableWidgetItem *___qtablewidgetitem3 = fileTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("musicplayerClass", "Time", 0));
        labelTotals->setText(QApplication::translate("musicplayerClass", "Total Size/Time:  ", 0));
        labelSize->setText(QApplication::translate("musicplayerClass", "000000 bytes", 0));
        labelTime->setText(QApplication::translate("musicplayerClass", "9h99m99s", 0));
        addFilesButton->setText(QApplication::translate("musicplayerClass", "Add...", 0));
        removeFilesButton->setText(QApplication::translate("musicplayerClass", "Remove", 0));
        selectAllButton->setText(QApplication::translate("musicplayerClass", "Select All", 0));
        upButton->setText(QApplication::translate("musicplayerClass", "Up", 0));
        downButton->setText(QApplication::translate("musicplayerClass", "Down", 0));
        label_2->setText(QApplication::translate("musicplayerClass", "Serial Port:", 0));
        comPortComboBox->clear();
        comPortComboBox->insertItems(0, QStringList()
         << QApplication::translate("musicplayerClass", "COM1", 0)
         << QApplication::translate("musicplayerClass", "COM2", 0)
         << QApplication::translate("musicplayerClass", "COM3", 0)
         << QApplication::translate("musicplayerClass", "COM4", 0)
         << QApplication::translate("musicplayerClass", "COM5", 0)
         << QApplication::translate("musicplayerClass", "COM6", 0)
         << QApplication::translate("musicplayerClass", "COM7", 0)
         << QApplication::translate("musicplayerClass", "COM8", 0)
         << QApplication::translate("musicplayerClass", "COM9", 0)
        );
        downloadButton->setText(QApplication::translate("musicplayerClass", "Download", 0));
    } // retranslateUi

};

namespace Ui {
    class musicplayerClass: public Ui_musicplayerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MUSICPLAYER_H
