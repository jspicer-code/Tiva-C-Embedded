#include "downloader.h"
#include "qfiledialog.h"
#include "qstandardpaths.h"
#include "qmessagebox.h"
#include "exporter.h"
#include "qserialport.h"
#include <qfileinfo.h>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <assert.h>

// Allows ExportSong* to be stored in QVariant.
Q_DECLARE_METATYPE(ExportSong*);

Downloader::Downloader(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.comPortComboBox->setCurrentText("COM8");

	connect(ui.addFilesButton, &QPushButton::clicked, this, &Downloader::addFilesButton_clicked);
	connect(ui.downloadButton, &QPushButton::clicked, this, &Downloader::downloadButton_clicked);
	connect(ui.selectAllButton, &QPushButton::clicked, this, &Downloader::selectAllButton_clicked);
	connect(ui.removeFilesButton, &QPushButton::clicked, this, &Downloader::removeFilesButton_clicked);
	connect(ui.upButton, &QPushButton::clicked, this, &Downloader::upButton_clicked);
	connect(ui.downButton, &QPushButton::clicked, this, &Downloader::downButton_clicked);
	connect(ui.fileTable, &QTableWidget::itemSelectionChanged, this, &Downloader::fileTable_itemSelectionChanged);

	ui.fileTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui.fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.upButton->setEnabled(false);
	ui.downButton->setEnabled(false);

	UpdateDisplayTotals();

}

void Downloader::fileTable_itemSelectionChanged()
{
	QItemSelectionModel *selectionModel = ui.fileTable->selectionModel();
	int count = selectionModel->selectedRows().count();
	ui.upButton->setEnabled(count == 1);
	ui.downButton->setEnabled(count == 1);
}


void Downloader::downloadButton_clicked(bool checked)
{
	int totalSize = (int)sizeof(SongAlbumHeader_t)+((int)sizeof(uint32_t)* ui.fileTable->rowCount());

	ExportAlbum album;
	for (int i = 0; i < ui.fileTable->rowCount(); i++) {
		QTableWidgetItem* item = ui.fileTable->item(i, 0);
		ExportSong* song = item->data(Qt::UserRole).value<ExportSong*>();
		album.files.push_back(song);
		totalSize += song->size;
	}

	if (album.files.size() == 0) {
		QMessageBox::critical(this, tr("MIDI Downloader"), tr("Add one or more MIDI files to the list."), QMessageBox::StandardButton::Ok);
		return;
	}
	else if (totalSize > SONG_MAX_ALBUM_SIZE) {
		QMessageBox::critical(this, tr("MIDI Downloader"), tr("The song list exceeds the maximum download size of 224Kib (229376 bytes).  Remove one or more files from the list."), QMessageBox::StandardButton::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	DownloadAlbum(&album);

	QApplication::restoreOverrideCursor();		
}

void Downloader::selectAllButton_clicked(bool checked)
{
	ui.fileTable->selectAll();
}

void Downloader::removeFilesButton_clicked(bool checked)
{
	while (ui.fileTable->selectedItems().count()) {
		int row = ui.fileTable->selectedItems().first()->row();
		
		QTableWidgetItem* item = ui.fileTable->item(row, 0);
		ExportSong* song = item->data(Qt::UserRole).value<ExportSong*>();
		delete song;
		
		item->setData(Qt::UserRole, 0);

		ui.fileTable->removeRow(row);
	}

	UpdateDisplayTotals();
}

void Downloader::addFilesButton_clicked(bool checked)
{

	QString defaultLocation;

	if (lastOpenFileLocation_.size()) {
		defaultLocation = lastOpenFileLocation_;
	}
	else {
		QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
		if (locations.count() > 0) {
			defaultLocation = locations[0];
		}
	}

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
		tr("Open Midi File"), defaultLocation, tr("Midi Files (*.mid *.midi)"));

	QApplication::setOverrideCursor(Qt::WaitCursor);
	qApp->processEvents();

	if (fileNames.count()) {

		// Save the last open file location 
		QFileInfo fileInfo(fileNames[0]);
		lastOpenFileLocation_ = fileInfo.absoluteDir().path();

		for (int i = 0; i < fileNames.count(); i++) {

			QFileInfo fileInfo(fileNames[i]);

			ExportSong* song = new ExportSong();
			song->name = fileInfo.fileName().toStdString();

			if (Exporter::ExportMidiFile(fileNames[i].toStdString().c_str(), song)) {
				delete song;
			}
			else {
				ui.fileTable->insertRow(ui.fileTable->rowCount());
				int row = ui.fileTable->rowCount() - 1;
				InsertRow(row, song, fileInfo.fileName());
			}
		}

		ui.fileTable->resizeColumnsToContents();

		UpdateDisplayTotals();

	}

	QApplication::restoreOverrideCursor();

}

void Downloader::upButton_clicked(bool checked)
{
	int row = ui.fileTable->selectedItems().first()->row();

	if (row - 1 >= 0) {
	
		std::vector<QTableWidgetItem*> cells;
		for (int i = 0; i < ui.fileTable->columnCount(); i++) {
			cells.push_back(ui.fileTable->takeItem(row, i));
		}

		ui.fileTable->removeRow(row);
		
		row--;
		ui.fileTable->insertRow(row);
		for (int i = 0; i < ui.fileTable->columnCount(); i++) {
			ui.fileTable->setItem(row, i, cells[i]);
		}
		
		ui.fileTable->selectRow(row);
	}
}


void Downloader::downButton_clicked(bool checked)
{

	int row = ui.fileTable->selectedItems().first()->row();

	if (row + 1 < ui.fileTable->rowCount()) {

		std::vector<QTableWidgetItem*> cells;
		for (int i = 0; i < ui.fileTable->columnCount(); i++) {
			cells.push_back(ui.fileTable->takeItem(row, i));
		}

		ui.fileTable->removeRow(row);

		row++;
		ui.fileTable->insertRow(row);
		for (int i = 0; i < ui.fileTable->columnCount(); i++) {
			ui.fileTable->setItem(row, i, cells[i]);
		}

		ui.fileTable->selectRow(row);
	}

}


void Downloader::UpdateDisplayTotals()
{
	int totalSize = 0;
	int totalTime = 0;
	for (int i = 0; i < ui.fileTable->rowCount(); i++) {
		QTableWidgetItem* item = ui.fileTable->item(i, 0);
		ExportSong* song = item->data(Qt::UserRole).value<ExportSong*>();

		totalSize += song->size;
		totalTime += song->duration;
	}

	if (totalSize > 0) {

		// Add the album header size.
		totalSize += (int)sizeof(SongAlbumHeader_t)+((int)sizeof(uint32_t)* ui.fileTable->rowCount());

		ui.labelTotals->setEnabled(true);
		ui.labelSize->setText(QString::number(totalSize).append(" bytes"));
		ui.labelTime->setText(QString("%1m%2s").arg(QString::number(totalTime / 60), QString::number(totalTime % 60)));

		if (totalSize > SONG_MAX_ALBUM_SIZE) {
			ui.labelSize->setStyleSheet("QLabel { background-color: red; }");
		}
		else {
			ui.labelSize->setStyleSheet("");
		}

	}
	else {
		ui.labelTotals->setEnabled(false);
		ui.labelSize->setText("");
		ui.labelTime->setText("");
	}

	this->update();
}

void Downloader::InsertRow(int row, ExportSong* song, const QString& fileName)
{
	// Store the pointer to the ExportSong in the the first column cell.
	QTableWidgetItem* item = new QTableWidgetItem(fileName);
	item->setData(Qt::UserRole, QVariant::fromValue<ExportSong*>(song));
	ui.fileTable->setItem(row, 0, item);

	item = new QTableWidgetItem(QString::number(song->size));
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui.fileTable->setItem(row, 1, item);

	item = new QTableWidgetItem(QString::number(song->tracks.size()).append(" (%1)").arg(song->midiTrackCount));
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui.fileTable->setItem(row, 2, item);

	int minutes = (int)(song->duration / 60);
	int seconds = (int)(song->duration % 60);
	item = new QTableWidgetItem(QString("%1m%2s").arg(QString::number(minutes), QString::number(seconds)));
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui.fileTable->setItem(row, 3, item);

}

bool Downloader::WaitForAck(QSerialPort* serial)
{

	char ack[2] = { '\0', '\0' };
	int bytesRead = 0;
	while (bytesRead < 2) {
		if (serial->waitForReadyRead(99999999)) {
			int count = serial->read(&ack[bytesRead], 2 - bytesRead);
			if (count < 0) {
				break;
			}
			bytesRead += count;
		}
		else {
			break;
		}
	}

	if (*(uint16_t*)&ack == 0xACCA) {
		return true;
	}

	return false;
}


void Downloader::SendStream(QSerialPort* serial, std::stringstream* stream, int size)
{
	stream->seekg(0, std::ios::end);
	assert((int)stream->tellg() == size);
	stream->seekg(0, std::ios::beg);

	char payload[1024];
	
	while (!stream->eof()) {

		int payloadSize = std::min(size - (int)stream->tellg(), 1024);
		if (payloadSize == 0) {
			// TODO:  sometimes eof is true and sometimes not when the position is equal to the length.??
			break;
		}

		stream->read(payload, payloadSize);

		QByteArray data;
		uint16_t code = 0xFEED;
		data.append((char*)&code, sizeof(uint16_t));
		data.append((char*)&payloadSize, sizeof(uint16_t));
		data.append(payload, payloadSize);

		int sent = serial->write(data);
		serial->flush();
		if (!WaitForAck(serial)) {
			break;
		}

	}
}

void Downloader::GenerateAlbumHeaderStream(const ExportAlbum& album, std::stringstream* stream)
{
	stream->str("");
	stream->clear();
	stream->seekp(0, std::ios::beg);

	SongAlbumHeader_t albumHeader;
	albumHeader.signature = SONG_ALBUM_SIGNATURE;
	albumHeader.numSongs = (uint16_t)album.files.size();
	stream->write(reinterpret_cast<const char*>(&albumHeader), sizeof(SongAlbumHeader_t));
	assert((size_t)stream->tellp() == sizeof(SongAlbumHeader_t));

	// Calculate the offset of the first song header, following the album header and song offset table.
	uint32_t songFileOffset = (uint32_t)stream->tellp() + (album.files.size() * sizeof(uint32_t));

	// Set each entry in the song offset table.
	for (size_t i = 0; i < album.files.size(); i++) {
		stream->write(reinterpret_cast<const char*>(&songFileOffset), sizeof(uint32_t));
		songFileOffset += album.files[i]->size;
	}
}

void Downloader::DownloadAlbum(ExportAlbum* album)
{

	std::stringstream stream;
	GenerateAlbumHeaderStream(*album, &stream);

	stream.seekp(0, std::ios::end);
	for (int i = 0; i < album->files.size(); i++) {
		ExportSong* song = album->files[i];
		song->stream.seekg(0, std::ios::beg);
		stream << song->stream.rdbuf();
	}
	
	int albumSize = (int)stream.tellp();

	stream.flush();
	
	QSerialPort serial(this);
	serial.setPortName(ui.comPortComboBox->currentText());
	serial.setBaudRate(115200);
	serial.setDataBits(QSerialPort::DataBits::Data8);
	serial.setParity(QSerialPort::Parity::NoParity);
	serial.setStopBits(QSerialPort::StopBits::OneStop);
	serial.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
	serial.setTextModeEnabled(false);

	if (serial.open(QIODevice::ReadWrite)) {

		QByteArray fileHeader;
		uint16_t code = 0xFEEF;
		uint16_t payloadSize = sizeof(uint32_t);
		uint32_t payload = albumSize;
		fileHeader.append((char*)&code, sizeof(uint16_t));
		fileHeader.append((char*)&payloadSize, sizeof(uint16_t));
		fileHeader.append((char*)&payload, sizeof(uint32_t));

		int sent = serial.write(fileHeader);
		serial.flush();
		if (WaitForAck(&serial)) {
			SendStream(&serial, &stream, albumSize);
		}

		serial.close();
	}

}
