#pragma once
#include <QtWidgets>
#include <QVector>
#include <algorithm>
#include <qelapsedtimer.h>
#include <QPoint>

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	QPainter* painter = nullptr;
	uchar* data = nullptr;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);
	QPoint drawLineEnd = QPoint(0, 0);

	QElapsedTimer timer;

	bool drawPolygonActivated = false;
	QVector<QPoint> drawPolygonPoints = QVector<QPoint>();

	bool drawCurveActivated = false;
	QVector<QPair<QPoint,QPoint>> drawCurveMasterPoints = QVector<QPair<QPoint,QPoint>>();

	bool croppedBySutherlandHodgman = false;

	//Image Editing variables
	bool dragReady = false;
	QPoint dragStartingPosition = QPoint();
	QPoint dragedPoint = QPoint();

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y) { return (x >= 0 && y >= 0 && x < img->width() && y < img->height()) ? true : false; }

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	void drawCircleBresenham(QPoint start, QPoint end, QColor color);
	void drawPolygon(QVector<QPoint> points, QColor color, int algType = 0, int fillingAlgType = 0);
	void scanLinePolygon(QVector<QPoint> points, QColor color);
	void fillTriangleSetup(QVector<QPoint> points, QColor color, int fillAlgType);
	void fillTriangle(QVector<QPoint> currentPoints,QVector<QPoint> oldPoints ,QColor color, int fillAlgType);
	QColor fillTriangleNearestNeighbour(QVector<QPoint> points,QPoint currentPoint, QVector<QColor> colors);
	QColor fillTriangleBaricentric(QVector<QPoint> points, QPoint currentPoint, QVector<QColor> colors);
	void drawCurve(QVector<QPair<QPoint, QPoint>> points, QColor color, int algType);
	void drawCurveHermint(QVector<QPair<QPoint, QPoint>> points, QColor color);
	void drawCurveCasteljau(QVector<QPoint> points, QColor color);
	void drawCurveCoons(QVector<QPoint> points, QColor color);

	QVector <QPoint> cyrusBeck(QPoint P1, QPoint P2);
	QVector <QPoint> sutherlandHodgman(QVector<QPoint> V);

	//Image Editing variables
	bool getDragReady() { return dragReady; }
	void setDragReady(bool state) { dragReady = state; }
	void setDragStartingPosition(QPoint start) { dragStartingPosition = start; }
	QPoint getDragStartingPosition() { return dragStartingPosition; }
	void setDragedPoint(QPoint point) { dragedPoint = point; }
	QPoint getDragedPoint() { return dragedPoint; }

	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img->bits(); }
	void setPainter() { painter = new QPainter(img); }

	int getImgWidth() { return img->width(); };
	int getImgHeight() { return img->height(); };

	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawLineEnd(QPoint end) { drawLineEnd = end; }
	QPoint getDrawLineEnd() { return drawLineEnd; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
	bool getDrawPolygonActivated() { return drawPolygonActivated; }
	QVector<QPoint>& getDrawPolygonPoints() { return drawPolygonPoints; }
	void setDrawPolygonPoints(QVector<QPoint> points) { drawPolygonPoints = points; }

	void setDrawCurveActivated(bool state) { drawCurveActivated = state; }
	bool getDrawCurveActivated() { return drawCurveActivated; }
	void setDrawCurveMasterPoints(QVector<QPair<QPoint, QPoint>> points) { drawCurveMasterPoints = points; }
	QVector<QPair<QPoint, QPoint>>& getDrawCurveMasterPoints() { return drawCurveMasterPoints; }
	

	void clear();

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};