/*
	WARNING: This code uses internal Qt API - the QZipReader class for reading
	ZIP files - and things may break if Qt changes the API. For Qt5 this is not
	a problem as we can "see the future" now and there are no changes in all
	the supported Qt5 versions up to the last one (5.15). In Qt6 the class
	might change or even disappear in the future, but this is very unlikely
	as there were no changes for several years and The Qt Company's policy
	is: "do not invest any resources into any desktop related stuff unless
	absolutely necessary". There is an issue (QTBUG-3897) since the year 2009 to
	include the ZIP reader into the public API, which aptly illustrates the
	effort The Qt Company is willing to make about anything desktop related...
*/

#include <QFileInfo>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QtEndian>
#include <QUrl>
#include <QRegularExpression>
#include <private/qzipreader_p.h>
#include "kmlparser.h"

static const QTemporaryDir &tempDir()
{
	static QTemporaryDir dir;
	return dir;
}

static bool isZIP(QFile *file)
{
	quint32 magic;

	return (file->read((char *)&magic, sizeof(magic)) == (qint64)sizeof(magic)
	  && qFromLittleEndian(magic) == 0x04034b50);
}

qreal KMLParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime KMLParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return d;
}

bool KMLParser::coord(Trackpoint &trackpoint)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (cp->isSpace() || cp->isNull()) {
			if (c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			if (c == 1) {
				trackpoint.setCoordinates(Coordinates(val[0], val[1]));
				if (!trackpoint.coordinates().isValid())
					return false;
			} else if (c == 2)
				trackpoint.setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			vp = cp;
			c++;
		}
	}

	return true;
}

bool KMLParser::pointCoordinates(Waypoint &waypoint)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (*cp == ',') {
			if (c > 1)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			waypoint.setCoordinates(Coordinates(val[0], val[1]));
			if (!waypoint.coordinates().isValid())
				return false;
			if (c == 2)
				waypoint.setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			c = 3;
		}
	}

	return true;
}

bool KMLParser::lineCoordinates(SegmentData &segment)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (*cp == ',') {
			if (c > 1)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1 || c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			segment.append(Trackpoint(Coordinates(val[0], val[1])));
			if (!segment.last().coordinates().isValid())
				return false;
			if (c == 2)
				segment.last().setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			c = 0;
			vp = cp;
		}
	}

 	return true;
}

bool KMLParser::polygonCoordinates(QVector<Coordinates> &points)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (*cp == ',') {
			if (c > 1)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1 || c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			points.append(Coordinates(val[0], val[1]));
			if (!points.last().isValid())
				return false;

			while (cp->isSpace())
				cp++;
			c = 0;
			vp = cp;
		}
	}

	return true;
}

QDateTime KMLParser::timeStamp()
{
	QDateTime ts;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("when"))
			ts = time();
		else
			_reader.skipCurrentElement();
	}

	return ts;
}

void KMLParser::lineString(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!lineCoordinates(segment))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::linearRing(QVector<Coordinates> &coordinates)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!polygonCoordinates(coordinates))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::boundary(QVector<Coordinates> &coordinates)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("LinearRing"))
			linearRing(coordinates);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::polygon(Area &area)
{
	Polygon polygon;

	while (_reader.readNextStartElement()) {
		QVector<Coordinates> path;

		if (_reader.name() == QLatin1String("outerBoundaryIs")) {
			if (!polygon.isEmpty()) {
				_reader.raiseError("Multiple polygon outerBoundaryIss");
				return;
			}
			boundary(path);
			polygon.append(path);
		} else if (_reader.name() == QLatin1String("innerBoundaryIs")) {
			if (polygon.isEmpty()) {
				_reader.raiseError("Missing polygon outerBoundaryIs");
				return;
			}
			boundary(path);
			polygon.append(path);
		} else
			_reader.skipCurrentElement();
	}

	area.append(polygon);
}

void KMLParser::point(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!pointCoordinates(waypoint))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}

	if (waypoint.coordinates().isNull())
		_reader.raiseError("Missing Point coordinates");
}

void KMLParser::heartRate(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Heartrate data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setHeartRate(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::cadence(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Cadence data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setCadence(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::speed(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Speed data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setSpeed(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::temperature(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Temperature data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setTemperature(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::schemaData(SegmentData &segment, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SimpleArrayData")) {
			QXmlStreamAttributes attr = _reader.attributes();
			QString name(attr.value("name").toString());

			if (name == QLatin1String("Heartrate"))
				heartRate(segment, start);
			else if (name == QLatin1String("Cadence"))
				cadence(segment, start);
			else if (name == QLatin1String("Speed"))
				speed(segment, start);
			else if (name == QLatin1String("Temperature"))
				temperature(segment, start);
			else
				_reader.skipCurrentElement();
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::extendedData(SegmentData &segment, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SchemaData"))
			schemaData(segment, start);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::track(SegmentData &segment)
{
	const char error[] = "gx:coord/when element count mismatch";
	int first = segment.size();
	int i = first;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("when")) {
			segment.append(Trackpoint());
			segment.last().setTimestamp(time());
		} else if (_reader.name() == QLatin1String("coord")) {
			if (i == segment.size()) {
				_reader.raiseError(error);
				return;
			} else if (!coord(segment[i])) {
				_reader.raiseError("Invalid coordinates");
				return;
			}
			i++;
		} else if (_reader.name() == QLatin1String("ExtendedData"))
			extendedData(segment, first);
		else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::multiTrack(TrackData &t)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Track")) {
			t.append(SegmentData());
			track(t.last());
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::multiGeometry(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints, const QString &name, const QString &desc,
  const QDateTime &timestamp)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			point(w);
		} else if (_reader.name() == QLatin1String("LineString")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			lineString(t.last());
		} else if (_reader.name() == QLatin1String("Polygon")) {
			areas.append(Area());
			Area &a = areas.last();
			a.setName(name);
			a.setDescription(desc);
			polygon(a);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::photoOverlay(const Ctx &ctx, QVector<Waypoint> &waypoints,
  QMap<QString, QPixmap> &icons)
{
	QString name, desc, phone, address, path, link, id;
	QDateTime timestamp;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			name = _reader.readElementText();
		else if (_reader.name() == QLatin1String("description"))
			desc = _reader.readElementText();
		else if (_reader.name() == QLatin1String("phoneNumber"))
			phone = _reader.readElementText();
		else if (_reader.name() == QLatin1String("address"))
			address = _reader.readElementText();
		else if (_reader.name() == QLatin1String("TimeStamp"))
			timestamp = timeStamp();
		else if (_reader.name() == QLatin1String("Style"))
			style(ctx.dir, icons);
		else if (_reader.name() == QLatin1String("Icon")) {
			QString image(icon());

			QRegularExpression re("\\$\\[[^\\]]+\\]");
			image.replace(re, "0");
			QUrl url(image);

			if (url.scheme() == "http" || url.scheme() == "https")
				link = image;
			else {
				if (ctx.zip) {
					if (tempDir().isValid()) {
						QFileInfo fi(image);
						QByteArray id(ctx.path.toUtf8() + image.toUtf8());
						path = tempDir().path() + "/" + QString("%0.%1").arg(
						  QCryptographicHash::hash(id, QCryptographicHash::Sha1)
						  .toHex(), QString(fi.suffix()));

						QFile::rename(ctx.dir.absoluteFilePath(image), path);
					}
				} else
					path = ctx.dir.absoluteFilePath(image);
			}
		} else if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			w.setAddress(address);
			w.setPhone(phone);
			w.setIcon(icons.value(id));
			if (!path.isNull())
				w.addImage(path);
			if (!link.isNull())
				w.addLink(Link(link, "Photo Overlay"));
			point(w);
		} else if (_reader.name() == QLatin1String("styleUrl")) {
			id = _reader.readElementText();
			id = (id.at(0) == '#') ? id.right(id.size() - 1) : QString();
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints,
  QMap<QString, QPixmap> &icons)
{
	QString name, desc, phone, address, id;
	QDateTime timestamp;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			name = _reader.readElementText();
		else if (_reader.name() == QLatin1String("description"))
			desc = _reader.readElementText();
		else if (_reader.name() == QLatin1String("phoneNumber"))
			phone = _reader.readElementText();
		else if (_reader.name() == QLatin1String("address"))
			address = _reader.readElementText();
		else if (_reader.name() == QLatin1String("TimeStamp"))
			timestamp = timeStamp();
		else if (_reader.name() == QLatin1String("Style"))
			style(ctx.dir, icons);
		else if (_reader.name() == QLatin1String("MultiGeometry"))
			multiGeometry(tracks, areas, waypoints, name, desc, timestamp);
		else if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			w.setAddress(address);
			w.setPhone(phone);
			w.setIcon(icons.value(id));
			point(w);
		} else if (_reader.name() == QLatin1String("LineString")
		  || _reader.name() == QLatin1String("LinearRing")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			lineString(t.last());
		} else if (_reader.name() == QLatin1String("Track")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			track(t.last());
		} else if (_reader.name() == QLatin1String("MultiTrack")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			multiTrack(t);
		} else if (_reader.name() == QLatin1String("Polygon")) {
			areas.append(Area());
			Area &a = areas.last();
			a.setName(name);
			a.setDescription(desc);
			polygon(a);
		} else if (_reader.name() == QLatin1String("styleUrl")) {
			id = _reader.readElementText();
			id = (id.at(0) == '#') ? id.right(id.size() - 1) : QString();
		} else
			_reader.skipCurrentElement();
	}
}

QString KMLParser::icon()
{
	QString path;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("href"))
			path = _reader.readElementText();
		else
			_reader.skipCurrentElement();
	}

	return path;
}

void KMLParser::iconStyle(const QDir &dir, const QString &id,
  QMap<QString, QPixmap> &icons)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Icon"))
			icons.insert(id, QPixmap(dir.absoluteFilePath(icon())));
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::style(const QDir &dir, QMap<QString, QPixmap> &icons)
{
	QString id = _reader.attributes().value("id").toString();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("IconStyle"))
			iconStyle(dir, id, icons);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::folder(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints,
  QMap<QString, QPixmap> &icons)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, icons);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	QMap<QString, QPixmap> icons;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, icons);
		else if (_reader.name() == QLatin1String("Style"))
			style(ctx.dir, icons);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	QMap<QString, QPixmap> icons;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, icons);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, icons);
		else
			_reader.skipCurrentElement();
	}
}

bool KMLParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	QFileInfo fi(*file);

	_reader.clear();

	if (isZIP(file)) {
		QZipReader zip(fi.absoluteFilePath(), QIODevice::ReadOnly);
		QTemporaryDir tempDir;
		if (!tempDir.isValid() || !zip.extractAll(tempDir.path()))
			_reader.raiseError("Error extracting ZIP file");
		else {
			QDir zipDir(tempDir.path());
			QFileInfoList files(zipDir.entryInfoList(QStringList("*.kml"),
			  QDir::Files));

			if (files.isEmpty())
				_reader.raiseError("No KML file found in ZIP file");
			else {
				QFile kmlFile(files.first().absoluteFilePath());
				if (!kmlFile.open(QIODevice::ReadOnly))
					_reader.raiseError("Error opening KML file");
				else {
					_reader.setDevice(&kmlFile);

					if (_reader.readNextStartElement()) {
						if (_reader.name() == QLatin1String("kml"))
							kml(Ctx(fi.absoluteFilePath(), zipDir, true),
							  tracks, areas, waypoints);
						else
							_reader.raiseError("Not a KML file");
					}
				}
			}
		}
	} else {
		file->reset();
		_reader.setDevice(file);

		if (_reader.readNextStartElement()) {
			if (_reader.name() == QLatin1String("kml"))
				kml(Ctx(fi.absoluteFilePath(), fi.absoluteDir(), false), tracks,
				  areas, waypoints);
			else
				_reader.raiseError("Not a KML file");
		}
	}

	return !_reader.error();
}
