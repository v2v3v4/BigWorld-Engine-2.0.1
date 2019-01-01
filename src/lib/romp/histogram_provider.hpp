/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HISTOGRAM_PROVIDER_HPP
#define HISTOGRAM_PROVIDER_HPP

/**
 * TODO: to be documented.
 */
class HistogramProvider
{
public:
	enum DUMMY
	{
		HISTOGRAM_LEVEL = 256
	};
	/**
	 * TODO: to be documented.
	 */
	struct Histogram
	{
		unsigned int value_[ 256 ];
	};
	enum Type
	{
		HT_R,
		HT_G,
		HT_B,
		HT_LUMINANCE,
		HT_NUM
	};
	HistogramProvider();
	void addUser();
	void removeUser();
	bool inUse() const;
	Histogram get( Type type ) const;
	void update();

	static HistogramProvider& instance();

private:
	unsigned int usageCount_;
	Histogram histogram_[ HT_NUM ];
};

#endif//HISTOGRAM_PROVIDER_HPP
