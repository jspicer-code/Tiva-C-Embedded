#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_musicplayer.h"
#include "exporter.h"

class QSerialPort;
class Downloader : public QMainWindow
{

	// For Error:  Unresolved external symbol "public: virtual struct QMetaObject const * __thiscall Parent
	//  If you're using Visual Studio, delete the line Q_OBJECT from the header file, save the file, put Q_OBJECT back into the header file, 
	//  save the file again. This should generate the moc_* file and should build and link correctly.
	//	https://stackoverflow.com/questions/14170770/unresolved-external-symbol-public-virtual-struct-qmetaobject-const-thiscal
	//
	Q_OBJECT

public:
	Downloader(QWidget *parent = Q_NULLPTR);

private:
	Ui::musicplayerClass ui;
	QString lastOpenFileLocation_;

	void DownloadAlbum(ExportAlbum* album);
	void GenerateAlbumHeaderStream(const ExportAlbum& album, std::stringstream* stream);
	void SendStream(QSerialPort* serial, std::stringstream* stream, int size);
	bool WaitForAck(QSerialPort* serial);
	
	void addFilesButton_clicked(bool checked = false);
	void downloadButton_clicked(bool checked);
	void selectAllButton_clicked(bool checked);
	void removeFilesButton_clicked(bool checked);
	void upButton_clicked(bool checked);
	void downButton_clicked(bool checked);
	void fileTable_itemSelectionChanged();

	void UpdateDisplayTotals();
	void InsertRow(int row, ExportSong* song, const QString& fileName);
};
