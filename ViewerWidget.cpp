#include   "ViewerWidget.h"
#include <QElapsedTimer>

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		size_t startbyte = y * img->bytesPerLine() + x * 4;

		data[startbyte] = color.blue();
		data[startbyte + 1] = color.green();
		data[startbyte + 2] = color.red();
		data[startbyte + 3] = color.alpha();
	}
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!croppedBySutherlandHodgman) {
		QVector<QPoint> newPoints = cyrusBeck(start, end);
		if (newPoints.isEmpty()) {
			qDebug() << "drawing line: none";
			return;
		}
		else {
			qDebug() << "drawing line: " << start << end;
			start = newPoints[0];
			end = newPoints[1];
		}
	}

	if (algType == 0) { //DDA
		drawLineDDA(start, end, color);
	}
	else if (algType == 1) { //Bresenham
		//drawLineBresenham(start, end, color);
		if (start.x() == end.x()) {
			if (start.y() > end.y()) {
				std::swap(start, end);
			}
			for (int y = start.y(); y <= end.y(); y++) {
				if (isInside(start.x(), y)) {
					setPixel(start.x(), y, color);
				}
			}
			return;
		}
		double m = (static_cast<double>(end.y()) - static_cast<double>(start.y())) / (static_cast<double>(end.x()) - static_cast<double>(start.x()));
		if (abs(m) <= 1) {  //riadiaca os X
			if (start.x() > end.x()) {
				std::swap(start, end);
			}
			int twoDeltaX = 2 * (end.x() - start.x());
			int twoDeltaY = 2 * (end.y() - start.y());
			if (0 < m && m <= 1) {
				int y = start.y();
				int pCurrent = twoDeltaY - twoDeltaX / 2;
				int k1 = twoDeltaY;
				int k2 = twoDeltaY - twoDeltaX;
				if (isInside(start.x(), start.y())) {
					setPixel(start.x(), start.y(), color);
				}
				for (int x = start.x(); x <= end.x(); x++) {
					if (pCurrent > 0) {
						y++;
						pCurrent += k2;
					}
					else {
						pCurrent += k1;
					}
					if (isInside(x, y)) {
						setPixel(x, y, color);
					}
				}
			}
			else {
				int y = start.y();
				int pCurrent = twoDeltaY + twoDeltaX / 2;
				int k1 = twoDeltaY;
				int k2 = twoDeltaY + twoDeltaX;
				if (isInside(start.x(), start.y())) {
					setPixel(start.x(), start.y(), color);
				}
				for (int x = start.x(); x <= end.x(); x++) {
					if (pCurrent < 0) {
						y--;
						pCurrent += k2;
					}
					else {
						pCurrent += k1;
					}
					if (isInside(x, y)) {
						setPixel(x, y, color);
					}
				}
			}
			
		}
		else { // riadiaca os Y
			if (start.y() > end.y()) {
				std::swap(start, end);
			}
			int twoDeltaX = 2 * (end.x() - start.x());
			int twoDeltaY = 2 * (end.y() - start.y());
			if (m > 1) {
				int x = start.x();
				int pCurrent = twoDeltaX - twoDeltaY / 2;
				int k1 = twoDeltaX;
				int k2 = twoDeltaX - twoDeltaY;
				if (isInside(start.x(), start.y())) {
					setPixel(start.x(), start.y(), color);
				}
				for (int y = start.y(); y <= end.y(); y++) {
					if (pCurrent > 0) {
						x++;
						pCurrent += k2;
					}
					else {
						pCurrent += k1;
					}
					if (isInside(x, y)) {
						setPixel(x, y, color);
					}
				}
			}
			else {
				int x = start.x();
				int pCurrent = twoDeltaX + twoDeltaY / 2;
				int k1 = twoDeltaX;
				int k2 = twoDeltaX + twoDeltaY;
				if (isInside(start.x(), start.y())) {
					setPixel(start.x(), start.y(), color);
				}
				for (int y = start.y(); y <= end.y(); y++) {
					if (pCurrent < 0) {
						x--;
						pCurrent += k2;
					}
					else {
						pCurrent += k1;
					}
					if (isInside(x, y)) {
						setPixel(x, y, color);
					}
				}
			}
			
		}
	}
	//drawCircleBresenham(start, start + QPoint(0, 2), Qt::red);
	//drawCircleBresenham(end, end + QPoint(0, 2), Qt::red);
	update();
}
void ViewerWidget::drawCircleBresenham(QPoint start, QPoint end, QColor color) {
	int r = static_cast<int>(sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2)));
	int x = 0, twoX = 3;
	int y = r, twoY = 2 * r - 2;
	int pCurrent = 1 - r;
	for (x = 0; x <= y; x++) {
		if (isInside(y + start.x(), x + start.y())) {
			setPixel(y + start.x(), x + start.y(), color);
		}
		if (isInside(x + start.x(), y + start.y())) {
			setPixel(x + start.x(), y + start.y(), color);
		}
		if (isInside(x + start.x(), -y + start.y())) {
			setPixel(x + start.x(), -y + start.y(), color);
		}
		if (isInside(-y + start.x(), x + start.y())) {
			setPixel(-y + start.x(), x + start.y(), color);
		}
		if (isInside(-y + start.x(), -x + start.y())) {
			setPixel(-y + start.x(), -x + start.y(), color);
		}
		if (isInside(-x + start.x(), -y + start.y())) {
			setPixel(-x + start.x(), -y + start.y(), color);
		}
		if (isInside(-x + start.x(), y + start.y())) {
			setPixel(-x + start.x(), y + start.y(), color);
		}
		if (isInside(y + start.x(), -x + start.y())) {
			setPixel(y + start.x(), -x + start.y(), color);
		}
		if (pCurrent > 0) {
			pCurrent = pCurrent - twoY;
			y--;
			twoY -= 2;
		}
		pCurrent += twoX;
		twoX += 2;
	}
	update();
}
void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color) {
	if (start.x() != end.x()) {
		double m = (static_cast<double>(end.y()) - static_cast<double>(start.y())) / (static_cast<double>(end.x()) - static_cast<double>(start.x()));
		if (abs(m) <= 1) { //riadiaca os X
			if (start.x() > end.x()) {
				std::swap(start, end);
			}
			double y = start.y();
			for (int x = start.x(); x < end.x(); x++) {
				if (isInside(x, static_cast<int>(y + 0.5))) {
					setPixel(x, static_cast<int>(y + 0.5), color);
				}
				y += m;
			}
		}
		else { //riadiaca os Y
			if (start.y() > end.y()) {
				std::swap(start, end);
			}
			double x = start.x();
			for (int y = start.y(); y < end.y(); y++) {
				if (isInside(static_cast<int>(x + 0.5), y)) {
					setPixel(static_cast<int>(x + 0.5), y, color);
				}
				x += 1 / m;
			}
		}
	}
	else {
		if (start.y() > end.y()) {
			std::swap(start, end);
		}
		for (int y = start.y(); y < end.y(); y++) {
			if (isInside(start.x(), y)) {
				setPixel(start.x(), y, color);
			}
		}
	}
}
void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color) {
	if (start.x() == end.x()) {
		if (start.y() > end.y()) {
			std::swap(start, end);
		}
		for (int y = start.y(); y <= end.y(); y++) {
			setPixel(start.x(), y, color);
		}
		return;
	}
	double m = (static_cast<double>(end.y()) - static_cast<double>(start.y())) / (static_cast<double>(end.x()) - static_cast<double>(start.x()));
	int tmp = 2;
	if (abs(m) <= 1) {  //riadiaca os X
		if (start.x() > end.x()) {
			std::swap(start, end);
		}
		int twoDeltaX = 2 * (end.x() - start.x());
		int twoDeltaY = 2 * (end.y() - start.y());
		if (m < 0 && m <= 1) {
			tmp = -1;
		}
		else {
			tmp = 1;
		}
		int y = start.y();
		int pCurrent = twoDeltaY + tmp * twoDeltaX / 2;
		int k1 = twoDeltaY;
		int k2 = twoDeltaY + tmp * twoDeltaX;
		setPixel(start.x(), start.y(), color);
		for (int x = start.x(); x <= end.x(); x++) {

			if (tmp == -1) {
				if (pCurrent > 0) {
					y ++;
					pCurrent += k2;
				}
				else {
					pCurrent += k1;
				}
			}
			else {
				if (pCurrent > 0) {
					y--;
					pCurrent += k2;
				}
				else {
					pCurrent += k1;
				}
			}
			
			setPixel(x, y, color);
		}

	}
	else { // riadiaca os Y
		if (start.y() > end.y()) {
			std::swap(start, end);
		}
		int twoDeltaX = 2 * (end.x() - start.x());
		int twoDeltaY = 2 * (end.y() - start.y());
		if (m > 1) {
			tmp = -1;
		}
		else {
			tmp = 1;
		}
		int x = start.x();
		int pCurrent = twoDeltaX + tmp * twoDeltaY / 2;
		int k1 = twoDeltaX;
		int k2 = twoDeltaX + tmp * twoDeltaY;
		setPixel(start.x(), end.x(), color);
		for (int y = start.y(); y <= end.y(); y++) {
			if (pCurrent > 0) {
				x -= tmp;
				pCurrent += k2;
			}
			else {
				pCurrent += k1;
			}
			setPixel(x, y, color);
		}
	}
}
void ViewerWidget::drawPolygon(QVector<QPoint> points, QColor color, int algType, int fillingAlgType) {
	QVector<QPoint> tmpPoints = sutherlandHodgman(points);
	if (tmpPoints.isEmpty() || points.length() <= 2) {
		qDebug() << "drawing polygon : none";
		return;
	}
	qDebug() << "drawing polygon : " << points;
	points = tmpPoints;
	croppedBySutherlandHodgman = true;
	if (fillingAlgType == 0) {
		for (int i = 0; i < points.length(); i++) {
			if (i < points.length() - 1) {
				drawLine(points[i], points[i + 1], color, algType);
			}
			else {
				drawLine(points[i], points[0], color, algType);
			}
		}
	}
	else if (points.length() == 3) {
		fillTriangleSetup(points, color, fillingAlgType);
	}
	else {
		scanLinePolygon(points, color);
	}
	croppedBySutherlandHodgman = false;
}
void ViewerWidget::scanLinePolygon(QVector<QPoint> points, QColor color) {
	struct Edge {											// structura obsahuje informacie o krivke
		QPoint start;
		QPoint end;
		int deltaY = 0;
		double m = 0;
	};
	QVector<Edge> e;
	QPoint start = points.last();
	QPoint end = QPoint();
	for (int i = 0; i < points.length(); i++) {
		end = points[i];
		if (start.y() > end.y()) {							// usporiadanie krivky z hora dole
			std::swap(start, end);
		}
		if (end.y() != start.y()) {							// vynechanie horizontalnych hran
			Edge line;
			line.start = start;
			line.end = QPoint(end.x(),end.y() - 1);			// skratenie hrany o 1 px
			line.deltaY = line.end.y() - line.start.y();
			if (line.end.x() == line.start.x()) {
				line.m = -DBL_MAX;							// prevencia pred delenim nulou
			}
			else {
				line.m = line.deltaY / static_cast<double>(line.end.x() - line.start.x());
			}
			if (line.m != 0) {
				e.append(line);
			}
		}
		start = points[i];
	}
	std::sort(e.begin(), e.end(), [](const Edge &line1, const Edge &line2) {	
		return line1.start.y() < line2.start.y();			// sortovanie hran podla ich zaciatocnej suradnice
		});
	int ymin = e[0].start.y();
	int ymax = ymin;
	int y = ymin;
	for (const Edge& element : e) {
		if (element.end.y() > ymax) {
			ymax = element.end.y();							// stanovenie Maximalnej y suradnice
		}
	}
	QVector<QVector<Edge>> edgeTable = QVector<QVector<Edge>>(ymax - ymin + 1);
	QVector<Edge> eActive;
	for (const Edge& element : e) {
		int tableIndex = element.start.y() - ymin;
		edgeTable[tableIndex].append(element);
	}
	for (int i = 0; i < edgeTable.length(); i++) {

		if (!edgeTable[i].isEmpty()) {
			eActive.append(edgeTable[i]);
		}
		std::sort(eActive.begin(), eActive.end(), [y](Edge line1, Edge line2) {
			return (y - line1.start.y()) / line1.m + line1.start.x() < (y - line2.start.y()) / line2.m + line2.start.x();
			});
		if (eActive.length() % 2 == 0) {
			for (int j = 0; j < eActive.length(); j += 2) {
				int xIntercept1 = static_cast<int>((y - eActive[j].start.y()) / eActive[j].m) + eActive[j].start.x();
				int xIntercept2 = static_cast<int>((y - eActive[j + 1].start.y()) / eActive[j + 1].m) + eActive[j + 1].start.x();
				if (bool state = xIntercept1 != xIntercept2) {
					for (int x = xIntercept1; x <= xIntercept2; x++) {
						if (isInside(x, y)) {
							setPixel(x, y, color);
						}
					}
				}
			}
		}
		for (int k = 0; k < eActive.length(); k++) {
			if (eActive[k].deltaY == 0) {
				eActive.removeAt(k);
				k--;
			}
			else {
				eActive[k].deltaY --;
			}
		}
		y++;
	}
	update();
}
void ViewerWidget::fillTriangleSetup(QVector<QPoint> points, QColor color,int fillAlgType) {
	QVector<QPoint> T = points;
	std::sort(T.begin(), T.end(), [](QPoint point1, QPoint point2) {
		if (point1.y() < point2.y() || point1.y() == point2.y() && point1.x() < point2.x()) {
			return TRUE;
		}
		else {
			return FALSE;
		}
		});
	if (T[0].y() == T[1].y() || T[1].y() == T[2].y()) {
		fillTriangle(T, points, color, fillAlgType);
		return;
	}
	double m = static_cast<double>(T[2].y() - T[0].y()) / (T[2].x() - T[0].x());
	QPoint P = QPoint((T[1].y() - T[0].y()) / m + T[0].x(), T[1].y());
	if (T[1].x() < P.x()) {
		fillTriangle({ T[0],T[1],P }, points,color,fillAlgType);
		fillTriangle({ T[1], P, T[2] }, points, color,fillAlgType);
	}
	else {
		fillTriangle({ T[0], P, T[1] }, points, color,fillAlgType);
		fillTriangle({ P,T[1],T[2] }, points, color,fillAlgType);
	}
}
void ViewerWidget::fillTriangle(QVector<QPoint> currentPoints, QVector<QPoint> oldPoints,QColor color ,int fillAlgType) {
	struct Edge {
		QPoint start;
		QPoint end;
		double m = 0;
	};
	QVector<QColor> colors = { Qt::red, Qt::blue,Qt::green };
	QVector<Edge> edges;
	QPoint start = currentPoints.last();
	for (int i = 0; i < currentPoints.length(); i++) {
		QPoint end = currentPoints[i];
		if (start.y() > end.y()) {
			std::swap(start, end);
		}
		if (start.y() != end.y()) {
			Edge edge;
			edge.start = start;
			edge.end = end;
			edge.m = static_cast<double>(end.y() - start.y()) / (end.x() - start.x());
			edges.append(edge);
		}
		start = currentPoints[i];
	}
	if (edges[0].end.x() != edges[1].end.x()) {
		std::sort(edges.begin(), edges.end(), [](Edge edge1, Edge edge2) {
			return edge1.end.x() < edge2.end.x();
			});
	}
	int ymin = edges[0].start.y();
	int ymax = edges[0].end.y();
	double x1 = edges[0].start.x();
	double x2 = edges[1].start.x();
	for (int y = ymin; y < ymax; y++) {
		if (x1 != x2) {
			for (int x = static_cast<int>(x1); x <= static_cast<int>(x2); x++) {
				if (fillAlgType == 2) {
					color = fillTriangleNearestNeighbour(oldPoints, QPoint(x, y), colors);
				}
				else if (fillAlgType == 3) {
					color = fillTriangleBaricentric(oldPoints, QPoint(x, y), colors);
				}
				setPixel(x, y, color);
			}
		}
		x1 += 1 / edges[0].m;
		x2 += 1 / edges[1].m;
	}
	for (int i = 0; i < currentPoints.length(); i++) {
		drawCircleBresenham(currentPoints[i], QPoint(currentPoints[i].x(), currentPoints[i].y() + 1), Qt::black);
	}
	update();
}
QColor ViewerWidget::fillTriangleNearestNeighbour(QVector<QPoint> points,QPoint currentPoint, QVector<QColor> colors) {
	QVector<double> distance = QVector<double>(3);
	for (int i = 0; i < points.length(); i++) {
		distance[i] = sqrt(pow(currentPoint.x() - points[i].x(), 2) + pow(currentPoint.y() - points[i].y(), 2));
	}
	if (distance[0] <= distance[1] && distance[0] <= distance[2]) {
		return colors[0];
	}
	else if (distance[1] <= distance[2] && distance[1] <= distance[0]) {
		return colors[1];
	}
	else if (distance[2] <= distance[1] && distance[2] <= distance[0]) {
		return colors[2];
	}
	return Qt::white;
}
QColor ViewerWidget::fillTriangleBaricentric(QVector<QPoint> points, QPoint currentPoint, QVector<QColor> colors) {
	QPoint P = currentPoint;
	QVector<QPoint> T = points;
	double lambda[3];
	lambda[0] = abs((T[1].x() - P.x()) * (T[2].y() - P.y()) - (T[1].y() - P.y()) * (T[2].x() - P.x()));
	lambda[0] /= abs(static_cast<double>(T[1].x() - T[0].x()) * (T[2].y() - T[0].y()) - (T[1].y() - T[0].y()) * (T[2].x() - T[0].x()));

	lambda[1] = abs((T[0].x() - P.x()) * (T[2].y() - P.y()) - (T[0].y() - P.y()) * (T[2].x() - P.x()));
	lambda[1] /= abs(static_cast<double>(T[1].x() - T[0].x()) * (T[2].y() - T[0].y()) - (T[1].y() - T[0].y()) * (T[2].x() - T[0].x()));

	lambda[2] = 1 - lambda[0] - lambda[1];

	double red = 0, green = 0, blue = 0;
	for (int i = 0; i < 3; i++) {
		red += lambda[i] * colors[i].red();
		green += lambda[i] * colors[i].green();
		blue += lambda[i] * colors[i].blue();
	}
	return QColor(static_cast<int> (red), static_cast<int> (green), static_cast<int> (blue), 255);
}
void ViewerWidget::drawCurve(QVector<QPair<QPoint, QPoint>> points, QColor color, int algType) {
	if (algType == 0) {
		drawCurveHermint(points, color);
	}
	else {
		QVector<QPoint> vectorOfPoints;
		for (int i = 0; i < points.length(); i++) {
			vectorOfPoints.append(points[i].first);
		}
		if (algType == 1) {
			drawCurveCasteljau(vectorOfPoints, color);
		}
		else {
			drawCurveCoons(vectorOfPoints, color);
		}
	}
}
void ViewerWidget::drawCurveHermint(QVector<QPair<QPoint, QPoint>> points, QColor color) {
	auto cubicPolynoms = [](double t) ->QVector<double> {
		return { 2 * pow(t,3) - 3 * pow(t,2) + 1 ,
				-2 * pow(t,3) + 3 * pow(t,2),
				 pow(t,3) - 2 * pow(t,2) + t,
				 pow(t,3) - pow(t,2) };
		};
	double deltaT = 0.05;
	int n = points.length();
	QVector<QPair<QPoint, QPoint>> P = points;
	QPoint Q0 = QPoint();
	QPoint Q1 = QPoint();
	int k = 0;
	for (int i = 1; i < n; i++) {
		Q0 = P[i - 1].first;
		for (double t = deltaT; t < 1; t += deltaT) {
			QVector<double> F = cubicPolynoms(t);
			Q1 = P[i - 1].first * F[0] + P[i].first * F[1] + P[i - 1].second * F[2] + P[i].second * F[3];
			drawLine(QPoint(static_cast<int> (Q0.x()),static_cast<int>(Q0.y())), QPoint(static_cast<int> (Q1.x()), static_cast<int>(Q1.y())), color, 1);
			Q0 = Q1;
			k++;
		}
		qDebug() << "interations :" << k;
		k = 0;
		drawLine(QPoint(static_cast<int> (Q0.x()), static_cast<int>(Q0.y())), P[i].first, color, 1);
	}
	for (int i = 0; i < P.length(); i++) {
		drawLine(P[i].first, P[i].second, Qt::red, 1);
		drawCircleBresenham(P[i].first, P[i].first + QPoint(0, 1), Qt::black);
	}
	update();
}
void ViewerWidget::drawCurveCasteljau(QVector<QPoint> points, QColor color) {
	int n = points.length();
	QVector<QVector<QPoint>> P(n);
	for (int i = 0; i < n; i++) {
		P[i] = QVector<QPoint>(n - i);
	}
	P[0] = points;
	double deltaT = 0.025;
	QPoint Q0 = P[0][0];
	QPoint Q1 = QPoint();
	for (double t = deltaT; t <= 1; t += deltaT) {
		for (int i = 1; i < n; i++) {
			for (int j = 0; j < n - i; j++) {
				P[i][j] = (1 - t) * P[i - 1][j] + t * P[i - 1][j + 1];
			}
		}
		Q1 = P[n - 1][0];
		drawLine(QPoint(static_cast<int> (Q0.x()), static_cast<int>(Q0.y())), QPoint(static_cast<int> (Q1.x()), static_cast<int>(Q1.y())), color, 1);
		Q0 = Q1;
	}
	drawLine(QPoint(static_cast<int> (Q0.x()), static_cast<int>(Q0.y())), P[0][n - 1], color, 1);
	for (int i = 0; i < n; i++) {
		drawCircleBresenham(points[i], points[i] + QPoint(0, 2), Qt::red);
	}
	update();
}
void ViewerWidget::drawCurveCoons(QVector<QPoint> points, QColor color) {
	auto cubicPolynoms = [](double t)->QVector<double> {						// Lambda funckia ktora vracia vektor hodnotu polynomov v case t
		return { -pow(t,3) / 6 + pow(t,2) / 2 - t / 2 + 1. / 6,
				  pow(t,3) / 2 - pow(t,2) + 2. / 3,
				 -pow(t,3) / 2 + pow(t,2) / 2 + t / 2 + 1. / 6,
				  pow(t,3) / 6 };
		};
	QVector<QPoint> P = points;
	int n = points.length();
	double deltaT = 0.05;
	QPoint Q0 = QPoint();
	QPoint Q1 = QPoint();
	for (int i = 3; i < n; i++) {
		QVector<double> B = cubicPolynoms(0);
		Q0 = P[i - 3] * B[0] + P[i - 2] * B[1] + P[i - 1] * B[2] + P[i] * B[3];
		if (i > 3) {
			drawLine(QPoint(static_cast<int> (Q1.x()), static_cast<int>(Q1.y())), QPoint(static_cast<int> (Q0.x()), static_cast<int>(Q0.y())) , color, 1);
		}
		for (double t = deltaT; t <= 1; t += deltaT) {
			B = cubicPolynoms(t);
			Q1 = P[i - 3] * B[0] + P[i - 2] * B[1] + P[i - 1] * B[2] + P[i] * B[3];
			drawLine(QPoint(static_cast<int> (Q0.x()), static_cast<int>(Q0.y())), QPoint(static_cast<int> (Q1.x()), static_cast<int>(Q1.y())), color, 1);
			Q0 = Q1;
		}
	}
	for (const QPoint& point : points) {
		drawCircleBresenham(point, point + QPoint(0, 2), Qt::red);
	}
	update();
}

QVector<QPoint> ViewerWidget::cyrusBeck(QPoint P1, QPoint P2) {
	if (isInside(P1.x(), P1.y()) || isInside(P2.x(), P2.y()) || true) {
		double tMin = 0;
		double tMax = 1;
		QPoint vectorD = P2 - P1;
		QPoint E[4] = { QPoint(0,0),
			QPoint(0,img->height()),
			QPoint(img->width(),img->height()),
			QPoint(img->width(),0) };
		for (int i = 0; i < 4; i++) {
			QPoint lineVector;
			if (i < 3) {
				lineVector = E[i + 1] - E[i];
			}
			else {
				lineVector = E[0] - E[i];
			}
			QPoint normalVector = QPoint(lineVector.y(), -lineVector.x());
			QPoint vectorW = P1 - E[i];
			int dotProductDN = vectorD.x() * normalVector.x() + vectorD.y() * normalVector.y();
			int dotProductWN = vectorW.x() * normalVector.x() + vectorW.y() * normalVector.y();
			if (dotProductDN != 0) {
				double t = -static_cast<double>(dotProductWN) / dotProductDN;
				if (dotProductDN > 0 && t <= 1) {
					tMin = std::max(t, tMin);
				}
				else if (dotProductDN < 0 && t >= 0) {
					tMax = std::min(t, tMax);
				}
			}
		}
		if (tMin == 0 && tMax == 1) {
			return { P1,P2 };
		}
		else if (tMin < tMax) {
			QPoint newP1 = P1 + (P2 - P1) * tMin;
			QPoint newP2 = P1 + (P2 - P1) * tMax;
			return { newP1, newP2 };
		}
	}
	return { P1, P2 };
}
QVector<QPoint> ViewerWidget::sutherlandHodgman(QVector<QPoint> V) {
	QVector<QPoint> W;
	QPoint S = V.last();
	int xMin = 0;
	int x[4] = {0, 0, -img->width(), -img->height()};				// inicializacia pola hodnot xmin pre jednotlive hrany orezavania
	for (int j = 0; j < 4; j++) {
		xMin = x[j];
		for (int i = 0; i < V.length(); i++) {
			if (V[i].x() >= xMin) {
				if (S.x() >= xMin) {
					if (!W.contains(V[i])) {
						W.append(V[i]);
					}
				}
				else {
					QPoint Intersect = QPoint(xMin,static_cast<int>( S.y() + (xMin - S.x()) * static_cast<double>(V[i].y() - S.y()) / (V[i].x() - S.x())));
					if (!W.contains(Intersect)) {
						W.append(Intersect);
					}
					if (!W.contains(V[i])) {
						W.append(V[i]);
					}
				}
			}
			else {
				if (S.x() >= xMin) {
					QPoint Intersect = QPoint(xMin, static_cast<int>(S.y() + (xMin - S.x()) * static_cast<double>(V[i].y() - S.y()) / (V[i].x() - S.x())));
					if (!W.contains(Intersect)) {
						W.append(Intersect);
					}
				}
			}
			S = V[i];
		}
		if (W.isEmpty()) {
			return QVector<QPoint>();
		}
		V = W;
		for (QPoint& point : V) {
			point = QPoint(point.y(), -point.x());
		}
		S = V.last();
		W.clear();
	}
	W = V;
	return W;
}

void ViewerWidget::clear()
{
	img->fill(Qt::white);
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}