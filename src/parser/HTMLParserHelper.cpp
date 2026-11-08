#include "HTMLParserHelper.h"
#include <QList>
#include <qtextstream.h>

xmlNodePtr HTMLParserHelper::SkipTo(const QString &title, xmlNodePtr node) {
	while (node) {
		// xmlNode::content is only set for text-like nodes. Use
		// xmlNodeGetContent() so this also works when the caller passes an
		// element node (for example <b>Platforms:</b>).
		xmlChar *rawContent = xmlNodeGetContent(node);
		if (rawContent) {
			const QString content = QString::fromUtf8(reinterpret_cast<const char *>(rawContent));
			xmlFree(rawContent);
			if (content.contains(title, Qt::CaseInsensitive)) {
				return node;
			}
		}
		node = node->next;
	}
	return nullptr;
}

unsigned long HTMLParserHelper::convertToKB(const QString &str) {
	QString trimmed = str.trimmed();
	QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);

	if (parts.size() < 2)
		return 0;

	bool ok = false;
	double value = parts[0].toDouble(&ok);
	if (!ok) {
		return 0;
	}

	const QString unit = parts[1].toUpper();

	if (unit == "TB") {
		return static_cast<unsigned long>(value * 1024 * 1024 * 1024);
	} else if (unit == "GB") {
		return static_cast<unsigned long>(value * 1024 * 1024);
	} else if (unit == "MB") {
		return static_cast<unsigned long>(value * 1024);
	} else if (unit == "KB") {
		return static_cast<unsigned long>(value);
	} else if (unit == "B" || unit == "BYTES") {
		return static_cast<unsigned long>(value / 1024.0);
	}

	return 0;
}

int HTMLParserHelper::convertToSeconds(const QString &time_str) {
	QStringList parts = time_str.trimmed().split(':');

	if (parts.size() == 2) {
		bool minutesOk = false;
		bool secondsOk = false;
		int minutes = parts[0].toInt(&minutesOk);
		int seconds = parts[1].toInt(&secondsOk);
		if (!minutesOk || !secondsOk) {
			return 0;
		}
		return (minutes * 60) + seconds;
	}

	if (parts.size() == 3) {
		bool hoursOk = false;
		bool minutesOk = false;
		bool secondsOk = false;
		int hours = parts[0].toInt(&hoursOk);
		int minutes = parts[1].toInt(&minutesOk);
		int seconds = parts[2].toInt(&secondsOk);
		if (!hoursOk || !minutesOk || !secondsOk) {
			return 0;
		}
		return (hours * 3600) + (minutes * 60) + seconds;
	}

	return 0;
}

QString HTMLParserHelper::cleanFormat(const QString &str) {
	QString cleanedStr = str;
	cleanedStr.remove('(');
	cleanedStr.remove(')');
	return cleanedStr;
}

int HTMLParserHelper::GetNextCount(xmlNodePtr node) {
	if (!node) {
		return 0;
	}

	int count = 0;
	while ((node = node->next)) {
		++count;
	}
	return count;
}

xmlNodePtr HTMLParserHelper::TraverseNext(xmlNodePtr In, int depth) {
	if (depth == 0 || !In) {
		return In;
	}

	if (depth > 0) {
		for (int i = 0; i < depth && In; ++i)
			In = In->next;
	} else {
		for (int i = 0; i < -depth && In; ++i)
			In = In->prev;
	}

	return In;
}

xmlNodePtr HTMLParserHelper::ToNextTag(xmlNodePtr In, const QString &tag) {
	while (In) {
		In = In->next;
		if (In && In->name) {
			QString name = QString::fromUtf8((const char *) In->name);
			if (name.contains(tag, Qt::CaseInsensitive)) {
				return In;
			}
		}
	}
	return nullptr;
}
