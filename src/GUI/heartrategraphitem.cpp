#include "tooltip.h"
#include "heartrategraphitem.h"

HeartRateGraphItem::HeartRateGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent)
{
	qreal sum = 0;

	for (int j = 1; j < graph.size(); j++)
		sum += graph.at(j).y() * (graph.at(j).s() - graph.at(j-1).s());
	_avg = sum/graph.last().s();

	setToolTip(toolTip());
}

QString HeartRateGraphItem::toolTip() const
{
	ToolTip tt;

	tt.insert(tr("Maximum"), QString::number(max(), 'f', 0)
	  + UNIT_SPACE + tr("bpm"));
	tt.insert(tr("Average"), QString::number(avg(), 'f', 0)
	  + UNIT_SPACE + tr("bpm"));

	return tt.toString();
}
