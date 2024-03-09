#ifndef GEOJSONPARSER_H
#define GEOJSONPARSER_H

#include <QJsonObject>
#include "parser.h"

class QJsonObject;
class QJsonArray;
class Projection;

class GeoJSONParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	enum Type {
		Unknown,
		Point,
		LineString,
		MultiPoint,
		Polygon,
		MultiLineString,
		MultiPolygon,
		GeometryCollection,
		Feature,
		FeatureCollection
	};

	Type type(const QJsonObject &json);
	bool crs(const QJsonObject &object, Projection &proj);
	bool point(const QJsonObject &object, const Projection &parent,
	  const QJsonValue &properties, Waypoint &waypoint);
	bool multiPoint(const QJsonObject &object, const Projection &parent,
	  const QJsonValue &properties, QVector<Waypoint> &waypoints);
	bool lineString(const QJsonObject &coordinates, const Projection &parent,
	  const QJsonValue &properties, TrackData &track);
	bool multiLineString(const QJsonObject &object, const Projection &proj,
	  const QJsonValue &properties, TrackData &track);
	bool polygon(const QJsonObject &object, const Projection &parent,
	  const QJsonValue &properties, Area &area);
	bool multiPolygon(const QJsonObject &object, const Projection &proj,
	  const QJsonValue &properties, Area &area);
	bool geometryCollection(const QJsonObject &json, const Projection &parent,
	  const QJsonValue &properties, QList<TrackData> &tracks,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	bool feature(const QJsonObject &json, const Projection &parent,
	  QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	bool featureCollection(const QJsonObject &object, const Projection &parent,
	  QList<TrackData> &tracks, QList<Area> &areas, QVector<Waypoint> &waypoints);

	QString _errorString;
};

#endif // GEOJSONPARSER_H
