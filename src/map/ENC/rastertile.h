#ifndef ENC_RASTERTILE_H
#define ENC_RASTERTILE_H

#include <QPixmap>
#include "common/range.h"
#include "map/projection.h"
#include "map/transform.h"
#include "mapdata.h"
#include "style.h"
#include "atlasdata.h"

class TextItem;

namespace ENC {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, const MapData *data, int zoom, const Range &zoomRange,
	  const QRect &rect, qreal ratio) :
		_proj(proj), _transform(transform), _style(style), _map(data), _atlas(0),
		_zoom(zoom), _zoomRange(zoomRange), _rect(rect), _ratio(ratio) {}
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, AtlasData *data, int zoom, const Range &zoomRange,
	  const QRect &rect, qreal ratio) :
		_proj(proj), _transform(transform), _style(style), _map(0), _atlas(data),
		_zoom(zoom), _zoomRange(zoomRange), _rect(rect), _ratio(ratio) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	struct SectorLight
	{
		SectorLight(const Coordinates &pos, Style::Color color, uint visibility,
		  double range, double start, double end) : pos(pos), color(color),
		  visibility(visibility), range(range), start(start), end(end) {}

		Coordinates pos;
		Style::Color color;
		uint visibility;
		double range;
		double start;
		double end;
	};

	typedef QMap<Coordinates, Style::Color> LightMap;
	typedef QSet<Coordinates> SignalSet;

	void fetchData(QList<MapData::Poly> &polygons, QList<MapData::Line> &lines,
	  QList<MapData::Point> &points);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	QPainterPath painterPath(const Polygon &polygon) const;
	QPolygonF polyline(const QVector<Coordinates> &path) const;
	QVector<QPolygonF> polylineM(const QVector<Coordinates> &path) const;
	QPolygonF tsslptArrow(const QPointF &p, qreal angle) const;
	QPointF centroid(const QVector<Coordinates> &polygon) const;
	void processPoints(QList<MapData::Point> &points,
	  QList<TextItem*> &textItems, QList<TextItem *> &lights,
	  QList<SectorLight> &sectorLights);
	void processLines(const QList<MapData::Line> &lines,
	  QList<TextItem*> &textItems);
	void drawArrows(QPainter *painter, const QList<MapData::Point> &points) const;
	void drawPolygons(QPainter *painter, const QList<MapData::Poly> &polygons) const;
	void drawLines(QPainter *painter, const QList<MapData::Line> &lines) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems) const;
	void drawSectorLights(QPainter *painter, const QList<SectorLight> &lights) const;

	static bool polyCb(MapData *data, void *context);
	static bool pointCb(MapData *data, void *context);

	Projection _proj;
	Transform _transform;
	const Style *_style;
	const MapData *_map;
	AtlasData *_atlas;
	int _zoom;
	Range _zoomRange;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
};

}

#endif // ENC_RASTERTILE_H
