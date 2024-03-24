#include "ImageViewer.h"
#include <cmath>

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));
	ui->scrollArea->setWidget(vW);
	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: #%1;").arg(globalColor.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);

}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	qDebug() << e->pos().x() << e->pos().y();
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked() && ui->toolButtonDrawLine->isEnabled())
	{
		if (w->getDrawLineActivated()) {
			w->setDrawLineEnd(e->pos());
			w->drawLine(w->getDrawLineBegin(),w->getDrawLineEnd(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawLineActivated(false);
			ui->toolButtonDrawCircle->setEnabled(false);
			ui->toolButtonDrawLine->setEnabled(false);
			ui->toolButtonDrawPolygon->setEnabled(false);
			ui->toolButtonDrawCurve->setEnabled(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked() && ui->toolButtonDrawCircle->isEnabled()) {
		if (w->getDrawLineActivated()) {
			w->setDrawLineEnd(e->pos());
			w->drawCircleBresenham(w->getDrawLineBegin(), w->getDrawLineEnd(), globalColor);
			w->setDrawLineActivated(false);
			ui->toolButtonDrawCircle->setEnabled(false);
			ui->toolButtonDrawLine->setEnabled(false);
			ui->toolButtonDrawPolygon->setEnabled(false);
			ui->toolButtonDrawCurve->setEnabled(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked() && ui->toolButtonDrawPolygon->isEnabled()) {
		w->getDrawPolygonPoints().append(e->pos());
		w->drawCircleBresenham(e->pos(), e->pos() + QPoint(0, 2), Qt::black);
		if (w->getDrawPolygonPoints().length() >= 3) {
			w->setDrawPolygonActivated(true);
		}
	}
	else if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked() && ui->toolButtonDrawPolygon->isEnabled()) {
		if (w->getDrawPolygonActivated()) {
			w->drawPolygon(w->getDrawPolygonPoints(), globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
			w->setDrawPolygonActivated(false);
			ui->toolButtonDrawCircle->setEnabled(false);
			ui->toolButtonDrawLine->setEnabled(false);
			ui->toolButtonDrawPolygon->setEnabled(false);
			ui->toolButtonDrawCurve->setEnabled(false);
		}
	}
	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawCurve->isChecked() && ui->toolButtonDrawCurve->isEnabled()){
		QPair<QPoint, QPoint> vector = { e->pos(),e->pos() - QPoint(0, 100) };
		w->getDrawCurveMasterPoints().append(vector);
		ui->comboBoxCurveAlg->setEnabled(false);
		if (w->getDrawCurveMasterPoints().length() >= 2 && ui->comboBoxCurveAlg->currentIndex() != 2) {
			w->setDrawCurveActivated(true);
		}
		else if (w->getDrawCurveMasterPoints().length() >= 4 && ui->comboBoxCurveAlg->currentIndex() == 2) {
			w->setDrawCurveActivated(true);
		}
	}
	else if (e->button() == Qt::RightButton && ui->toolButtonDrawCurve->isChecked() && ui->toolButtonDrawCurve->isEnabled()) {
		if (w->getDrawCurveActivated()) {
			w->drawCurve(w->getDrawCurveMasterPoints(), globalColor, ui->comboBoxCurveAlg->currentIndex());
			w->setDrawCurveActivated(false);
			ui->toolButtonDrawCircle->setEnabled(false);
			ui->toolButtonDrawLine->setEnabled(false);
			ui->toolButtonDrawPolygon->setEnabled(false);
			ui->toolButtonDrawCurve->setEnabled(false);
		}
	}

	//Edit Position
	else if (ui->toolButtonEditPosition->isChecked() && e->button() == Qt::LeftButton) {
		w->setDragReady(true);
		w->setDragStartingPosition(e->pos());
		qDebug() << "drag ready";
		if(ui->toolButtonDrawCurve->isChecked() && !ui->toolButtonDrawCurve->isEnabled()){
			QVector<QPoint> points;
			for (const QPair<QPoint, QPoint>& pair : w->getDrawCurveMasterPoints()) {
				points.append({pair.first, pair.second});
			}
			QPoint P = e->pos();
			std::sort(points.begin(), points.end(), [P](const QPoint point1, const QPoint point2) {
				return sqrt(pow(point1.x() - P.x(), 2) + pow(point1.y() - P.y(), 2)) < sqrt(pow(point2.x() - P.x(), 2) + pow(point2.y() - P.y(), 2));
				});
			w->setDragedPoint(points.first());
		}
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (ui->toolButtonEditPosition->isChecked() && e->button() == Qt::LeftButton) {
		qDebug() << "coords of the object changed";
		w->setDragReady(false);
		QPoint delta = w->getDragStartingPosition() - e->pos();
		if (ui->toolButtonDrawCircle->isChecked() || ui->toolButtonDrawLine->isChecked()) {
			w->setDrawLineBegin(w->getDrawLineBegin() - delta);
			w->setDrawLineEnd(w->getDrawLineEnd() - delta);
		}
		else if (ui->toolButtonDrawPolygon->isChecked()) {
			QVector<QPoint> resultPolygon;
			for (const QPoint& point : w->getDrawPolygonPoints()) {
				resultPolygon.append(point - delta);
			}
			w->getDrawPolygonPoints() = resultPolygon;
		}
		else if (ui->toolButtonDrawCurve->isChecked()) {
			for (QPair<QPoint, QPoint>& pair : w->getDrawCurveMasterPoints()) {
				if (w->getDragedPoint() == pair.first) {
					pair.first -= delta;
					break;
				}
				else if (w->getDragedPoint() == pair.second) {
					pair.second -= delta;
					break;
				}
			}
			w->setDragedPoint(QPoint());
		}
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (ui->toolButtonEditPosition->isChecked() && w->getDragReady()) {
		QPoint delta = w->getDragStartingPosition() - e->pos();
		w->clear();
		if (ui->toolButtonDrawLine->isChecked()) {
			w->drawLine(w->getDrawLineBegin() - delta, w->getDrawLineEnd() - delta, globalColor, ui->comboBoxLineAlg->currentIndex());
		}
		else if (ui->toolButtonDrawCircle->isChecked()) {
			w->drawCircleBresenham(w->getDrawLineBegin() - delta, w->getDrawLineEnd() - delta, globalColor);
		}
		else if (ui->toolButtonDrawPolygon->isChecked()) {
			QVector<QPoint> resultPolygon;
			for (const QPoint& point : w->getDrawPolygonPoints()) {
				resultPolygon.append(point-delta);
			}
			w->drawPolygon(resultPolygon, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
		}
		else if (ui->toolButtonDrawCurve->isChecked()) {
			QVector<QPair<QPoint, QPoint>> pointTmp = w->getDrawCurveMasterPoints();
			for (QPair<QPoint, QPoint>& pair :pointTmp) {
				if (w->getDragedPoint() == pair.first) {
					pair.first -= delta;
					break;
				}
				else if (w->getDragedPoint() == pair.second) {
					pair.second -= delta;
					break;
				}
			}
			w->drawCurve(pointTmp, globalColor, ui->comboBoxCurveAlg->currentIndex());
		}
	}
	w->update();
}

void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	qDebug() << event->type();
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
	if (ui->toolButtonEditScale->isChecked()) {
		double value = 1;
		if (wheelEvent->angleDelta().y() > 0) { //scroll up
			value = 1.25;
		}
		else if(wheelEvent->angleDelta().y() < 0){
			value = 0.75;
		}
		qDebug() << wheelEvent->angleDelta().y();
		if (ui->toolButtonDrawLine->isChecked()) {
			int x = static_cast<int>((vW->getDrawLineEnd().x() - vW->getDrawLineBegin().x()) * value + vW->getDrawLineBegin().x() + 0.5);
			int y = static_cast<int>((vW->getDrawLineEnd().y() - vW->getDrawLineBegin().y()) * value + vW->getDrawLineBegin().y() + 0.5);
			qDebug() << x << y;
			vW->setDrawLineEnd(QPoint(x, y));
			vW->clear();
			vW->drawLine(vW->getDrawLineBegin(), vW->getDrawLineEnd(), globalColor, ui->comboBoxLineAlg->currentIndex());
		}
		else if (ui->toolButtonDrawPolygon->isChecked()) {
			QVector <QPoint> scaledPoints;
			scaledPoints.append(vW->getDrawPolygonPoints()[0]);
			for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
				int x = static_cast<int>((vW->getDrawPolygonPoints()[i].x() - vW->getDrawPolygonPoints()[0].x()) * value + vW->getDrawPolygonPoints()[0].x() + 0.5);
				int y = static_cast<int>((vW->getDrawPolygonPoints()[i].y() - vW->getDrawPolygonPoints()[0].y()) * value + vW->getDrawPolygonPoints()[0].y() + 0.5);
				QPoint point(x,y);
				scaledPoints.append(point);
			}
			qDebug() << vW->getDrawPolygonPoints().length();
			vW->setDrawPolygonPoints(scaledPoints);
			vW->clear();
			vW->drawPolygon(vW->getDrawPolygonPoints(), globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
		}
		vW->update();
	}
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_toolButtonDrawLine_clicked() {
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonDrawCurve->setChecked(false);
	ui->spinBoxRotation->setEnabled(false);
}
void ImageViewer::on_toolButtonDrawCircle_clicked() {
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonDrawCurve->setChecked(false);
	ui->spinBoxRotation->setEnabled(false);
}
void ImageViewer::on_toolButtonDrawPolygon_clicked() {
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
	ui->spinBoxRotation->setEnabled(false);
	ui->toolButtonDrawCurve->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
}
void ImageViewer::on_toolButtonDrawCurve_clicked() {
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonDrawCircle->setChecked(false);
	ui->spinBoxRotation->setEnabled(false);
}

void ImageViewer::on_toolButtonEditPosition_clicked() {
	ui->toolButtonEditRotation->setChecked(false);
	ui->toolButtonEditShear->setChecked(false);
	ui->spinBoxRotation->setEnabled(false);
	ui->toolButtonEditScale->setChecked(false);
	ui->spinBoxScaleX->setEnabled(false);
	ui->spinBoxScaleY->setEnabled(false);
	ui->spinBoxShearFactor->setEnabled(false);
}

void ImageViewer::on_toolButtonEditRotation_clicked() {
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonEditScale->setChecked(false);
	ui->toolButtonEditShear->setChecked(false);
	ui->spinBoxScaleX->setEnabled(false);
	ui->spinBoxScaleY->setEnabled(false);
	ui->spinBoxShearFactor->setEnabled(false);
	if (ui->toolButtonEditRotation->isChecked()) {
		ui->spinBoxRotation->setEnabled(true);
	}
	else {
		ui->spinBoxRotation->setEnabled(false);
	}
}
void ImageViewer::on_spinBoxRotation_valueChanged(int value) {
	double valueRadian = value * M_PI / 180;
	if (ui->toolButtonDrawLine->isChecked()) {
		QPoint delta = vW->getDrawLineEnd() - vW->getDrawLineBegin();
		int x = static_cast<int>(delta.x() * cos(valueRadian) + delta.y() * sin(valueRadian)) + vW->getDrawLineBegin().x();
		int y = static_cast<int>(-delta.x() * sin(valueRadian) + delta.y() * cos(valueRadian)) + vW->getDrawLineBegin().y();
		vW->clear();
		vW->drawLine(vW->getDrawLineBegin(), QPoint(x,y), globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector<QPoint> rotatedPoints;
		rotatedPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			QPoint delta = vW->getDrawPolygonPoints()[i] - vW->getDrawPolygonPoints()[0];
			int x = static_cast<int>(delta.x() * cos(valueRadian) + delta.y() * sin(valueRadian)) + vW->getDrawPolygonPoints()[0].x();
			int y = static_cast<int>(-delta.x() * sin(valueRadian) + delta.y() * cos(valueRadian)) + vW->getDrawPolygonPoints()[0].y();
			rotatedPoints.append(QPoint(x, y));
		}
		vW->clear();
		vW->drawPolygon(rotatedPoints, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
	}
	vW->update();
}
void ImageViewer::on_spinBoxRotation_editingFinished() {
	int value = ui->spinBoxRotation->value();
	double valueRadian = value * M_PI / 180;
	if (ui->toolButtonDrawLine->isChecked()) {
		QPoint delta = vW->getDrawLineEnd() - vW->getDrawLineBegin();
		int x = static_cast<int>(delta.x() * cos(valueRadian) + delta.y() * sin(valueRadian)) + vW->getDrawLineBegin().x();
		int y = static_cast<int>(-delta.x() * sin(valueRadian) + delta.y() * cos(valueRadian)) + vW->getDrawLineBegin().y();
		vW->setDrawLineEnd(QPoint(x, y));
		ui->spinBoxRotation->setValue(0);
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector<QPoint> rotatedPoints;
		rotatedPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			QPoint delta = vW->getDrawPolygonPoints()[i] - vW->getDrawPolygonPoints()[0];
			int x = static_cast<int>(delta.x() * cos(valueRadian) + delta.y() * sin(valueRadian)) + vW->getDrawPolygonPoints()[0].x();
			int y = static_cast<int>(-delta.x() * sin(valueRadian) + delta.y() * cos(valueRadian)) + vW->getDrawPolygonPoints()[0].y();
			rotatedPoints.append(QPoint(x, y));
		}
		vW->setDrawPolygonPoints(rotatedPoints);
		ui->spinBoxRotation->setValue(0);
	}
}

void ImageViewer::on_toolButtonEditScale_clicked() {
	if (ui->toolButtonEditScale->isChecked()) {
		ui->spinBoxScaleX->setEnabled(true);
		ui->spinBoxScaleY->setEnabled(true);
	}
	else {
		ui->spinBoxScaleX->setEnabled(false);
		ui->spinBoxScaleY->setEnabled(false);
	}

	ui->spinBoxRotation->setEnabled(false);
	ui->spinBoxShearFactor->setEnabled(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonEditRotation->setChecked(false);
	ui->toolButtonEditShear->setChecked(false);
}
void ImageViewer::on_spinBoxScaleX_valueChanged(double value) {
	if (ui->toolButtonDrawLine->isChecked()) {
		int x = static_cast<int>((vW->getDrawLineEnd().x() - vW->getDrawLineBegin().x()) * value + vW->getDrawLineBegin().x());
		vW->clear();
		vW->drawLine(vW->getDrawLineBegin(), QPoint(x, vW->getDrawLineEnd().y()), globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector <QPoint> scaledPoints;
		scaledPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			int x = static_cast<int>((vW->getDrawPolygonPoints()[i].x() - vW->getDrawPolygonPoints()[0].x()) * value + vW->getDrawPolygonPoints()[0].x());
			QPoint point(x, vW->getDrawPolygonPoints()[i].y());
			scaledPoints.append(point);
		}
		vW->clear();
		vW->drawPolygon(scaledPoints, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
	}
	vW->update();
}
void ImageViewer::on_spinBoxScaleY_valueChanged(double value) {
	if (ui->toolButtonDrawLine->isChecked()) {
		int y = static_cast<int>((vW->getDrawLineEnd().y() - vW->getDrawLineBegin().y()) * value + vW->getDrawLineBegin().y());
		vW->clear();
		vW->drawLine(vW->getDrawLineBegin(), QPoint(vW->getDrawLineEnd().x(),y), globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector <QPoint> scaledPoints;
		scaledPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 0; i < vW->getDrawPolygonPoints().length(); i++) {
			int y = static_cast<int>((vW->getDrawPolygonPoints()[i].y() - vW->getDrawPolygonPoints()[0].y()) * value + vW->getDrawPolygonPoints()[0].y());
			qDebug() << vW->getDrawPolygonPoints()[i].x();
			QPoint point( vW->getDrawPolygonPoints()[i].x(),y);
			scaledPoints.append(point);
		}
		vW->clear();
		vW->drawPolygon(scaledPoints, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
	}
	vW->update();
}
void ImageViewer::on_spinBoxScaleX_editingFinished() {
	double value = ui->spinBoxScaleX->value();
	if (ui->toolButtonDrawLine->isChecked()) {
		int x = static_cast<int>((vW->getDrawLineEnd().x() - vW->getDrawLineBegin().x()) * value + vW->getDrawLineBegin().x());
		vW->setDrawLineEnd(QPoint(x, vW->getDrawLineEnd().y()));
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector <QPoint> scaledPoints;
		scaledPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			int x = static_cast<int>((vW->getDrawPolygonPoints()[i].x() - vW->getDrawPolygonPoints()[0].x()) * value + vW->getDrawPolygonPoints()[0].x());
			QPoint point(x, vW->getDrawPolygonPoints()[i].y());
			scaledPoints.append(point);
		}
		vW->setDrawPolygonPoints(scaledPoints);
		ui->spinBoxScaleX->setValue(1);
	}
}
void ImageViewer::on_spinBoxScaleY_editingFinished() {
	double value = ui->spinBoxScaleY->value();
	if (ui->toolButtonDrawLine->isChecked()) {
		int y = static_cast<int>((vW->getDrawLineEnd().y() - vW->getDrawLineBegin().y()) * value + vW->getDrawLineBegin().y());
		vW->setDrawLineEnd(QPoint(vW->getDrawLineEnd().x(), y));
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector <QPoint> scaledPoints;
		scaledPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 0; i < vW->getDrawPolygonPoints().length(); i++) {
			int y = static_cast<int>((vW->getDrawPolygonPoints()[i].y() - vW->getDrawPolygonPoints()[0].y()) * value + vW->getDrawPolygonPoints()[0].y());
			QPoint point(vW->getDrawPolygonPoints()[i].x(), y);
			scaledPoints.append(point);
		}
		vW->setDrawPolygonPoints(scaledPoints);
		ui->spinBoxScaleY->setValue(1);
	}
}

void ImageViewer::on_toolButtonEditShear_clicked() {
	if (ui->toolButtonEditShear->isChecked()) {
		ui->spinBoxShearFactor->setEnabled(true);
	}
	else {
		ui->spinBoxShearFactor->setEnabled(false);
	}
	ui->toolButtonEditRotation->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonEditScale->setChecked(false);
	ui->spinBoxScaleX->setEnabled(false);
	ui->spinBoxScaleY->setEnabled(false);
	ui->spinBoxRotation->setEnabled(false);
}
void ImageViewer::on_spinBoxShearFactor_valueChanged(double value) {
	if (ui->toolButtonDrawLine->isChecked()) {
		int x = vW->getDrawLineEnd().x() + vW->getDrawLineEnd().y() * value;
		vW->clear();
		vW->drawLine(vW->getDrawLineBegin(), QPoint(x, vW->getDrawLineEnd().y()), globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector<QPoint> shearedPoints;
		shearedPoints.append(vW->getDrawPolygonPoints().first());
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			int x = vW->getDrawPolygonPoints()[i].x() + vW->getDrawPolygonPoints()[i].y() * value;
			shearedPoints.append(QPoint(x, vW->getDrawPolygonPoints()[i].y()));
		}
		vW->clear();
		vW->drawPolygon(shearedPoints, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
	}
	vW->update();
}
void ImageViewer::on_spinBoxShearFactor_editingFinished() {
	double value = ui->spinBoxShearFactor->value();
	if (ui->toolButtonDrawLine->isChecked()) {
		int x = vW->getDrawLineEnd().x() + vW->getDrawLineEnd().y() * value;
		vW->setDrawLineEnd(QPoint(x, vW->getDrawLineEnd().y()));
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QVector<QPoint> shearedPoints;
		shearedPoints.append(vW->getDrawPolygonPoints()[0]);
		for (int i = 1; i < vW->getDrawPolygonPoints().length(); i++) {
			int x = vW->getDrawPolygonPoints()[i].x() + vW->getDrawPolygonPoints()[i].y() * value;
			shearedPoints.append(QPoint(x, vW->getDrawPolygonPoints()[i].y()));
		}
		vW->setDrawPolygonPoints(shearedPoints);
	}
	ui->spinBoxShearFactor->setValue(0);
}

void ImageViewer::on_toolButtonSymmetry_clicked() {
	if (ui->toolButtonDrawLine->isChecked()) {
		QPoint A = QPoint(vW->getDrawLineBegin().x(), 0);
		QPoint B = QPoint(vW->getDrawLineBegin().x(), vW->getImgHeight());
		QPoint vectorAB = B - A;
		QPoint normAB = QPoint(vectorAB.y(), -vectorAB.x());
		int a = normAB.x();
		int b = normAB.y();		
		int c = -a * A.x() - b * A.y();
		int newX = static_cast<int>(vW->getDrawLineEnd().x() - 2 * a * (static_cast<double>(a * vW->getDrawLineEnd().x() + b * vW->getDrawLineEnd().y() + c) / (a * a + b * b)));
		int newY = static_cast<int>(vW->getDrawLineEnd().y() - 2 * b * (static_cast<double>(a * vW->getDrawLineEnd().x() + b * vW->getDrawLineEnd().y() + c) / (a * a + b * b)));
		vW->setDrawLineEnd(QPoint(newX, newY));
		vW->clear();
		vW->drawLine(vW->getDrawLineBegin(), vW->getDrawLineEnd(), globalColor, ui->comboBoxLineAlg->currentIndex());
		vW->update();
	}
	else if (ui->toolButtonDrawPolygon->isChecked()) {
		QPoint A = vW->getDrawPolygonPoints()[vW->getDrawPolygonPoints().length() - 1];
		QPoint B = vW->getDrawPolygonPoints()[0];
		QPoint vectorAB = B - A;
		QPoint normAB = QPoint(vectorAB.y(), -vectorAB.x());
		int a = normAB.x();
		int b = normAB.y();
		int c = -a * A.x() - b * A.y();
		QVector<QPoint> transformedPoints;
		transformedPoints.append(B);
		for (int i = 1; i < vW->getDrawPolygonPoints().length() - 1; i++) {
			int newX = static_cast<int>(vW->getDrawPolygonPoints()[i].x() - 2 * a * (static_cast<double>(a * vW->getDrawPolygonPoints()[i].x() + b * vW->getDrawPolygonPoints()[i].y() + c) / (a * a + b * b)));
			int newY = static_cast<int>(vW->getDrawPolygonPoints()[i].y() - 2 * b * (static_cast<double>(a * vW->getDrawPolygonPoints()[i].x() + b * vW->getDrawPolygonPoints()[i].y() + c) / (a * a + b * b)));
			transformedPoints.append(QPoint(newX, newY));
		}
		transformedPoints.append(A);
		vW->setDrawPolygonPoints(transformedPoints);
		vW->clear();
		vW->drawPolygon(transformedPoints, globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxFillingAlg->currentIndex());
		vW->update();
	}
}

void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	ui->toolButtonDrawCircle->setEnabled(true);
	ui->toolButtonDrawLine->setEnabled(true);
	ui->toolButtonDrawPolygon->setEnabled(true);
	ui->toolButtonDrawCurve->setEnabled(true);
	ui->comboBoxCurveAlg->setEnabled(true);
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawCurve->setChecked(false);
	ui->toolButtonEditPosition->setChecked(false);
	ui->toolButtonEditRotation->setChecked(false);
	ui->spinBoxRotation->setValue(0);
	ui->spinBoxRotation->setEnabled(false);
	ui->spinBoxScaleX->setValue(1);
	ui->spinBoxScaleY->setValue(1);
	ui->spinBoxScaleX->setEnabled(false);
	ui->spinBoxScaleY->setEnabled(false);
	vW->clear();
	vW->getDrawPolygonPoints().clear();
	vW->setDrawLineBegin(QPoint());
	vW->setDrawLineEnd(QPoint());
	vW->getDrawCurveMasterPoints().clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}