#include "core/Core.h"

QString stringifyMilliseconds(unsigned long long milliseconds)
{
	unsigned long long seconds = milliseconds / 1000;
	milliseconds %= 1000;
	unsigned long long minutes = seconds / 60;
	seconds %= 60;
	unsigned long long hours = minutes / 60;
	minutes %= 60;
	unsigned long long days = hours / 24;
	hours %= 24;

	QString result = "";
	if (days != 0)
	{
		result += QString("%1 days, ").arg(days);
	}
	if (hours != 0 || days != 0)
	{
		result += QString("%1 hours, ").arg(hours);
	}
	if (minutes != 0 || (days + hours) != 0)
	{
		result += QString("%1 minutes, ").arg(minutes);
	}
	if (seconds != 0 || (days + hours + minutes) != 0)
	{
		result += QString("%1 seconds").arg(seconds);
	}

	return result;
}
