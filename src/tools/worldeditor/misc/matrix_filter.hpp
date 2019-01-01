/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATRIX_FILTER_HPP
#define MATRIX_FILTER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *  Manages WE's terrain height filters 
 */
class MatrixFilter
{
public:
	struct FilterDef
	{
		FilterDef();
		void clear();

		float				constant_;
		float				noise_;
		int					noiseSizeX_;
		int					noiseSizeY_;
		float				strengthRatio_;
		std::vector<float>	kernel_;
		int					kernelWidth_;
		int					kernelHeight_;
		float				kernelSum_;
		bool				included_;
		std::string			name_;
	};

	static MatrixFilter& instance();

	size_t size() const;

	const FilterDef& filter( size_t index ) const;

private:
	MatrixFilter();
	bool init();

	bool inited_;

	typedef std::vector<FilterDef> FilterDefVec;
	FilterDefVec filters_;
};


#endif // MATRIX_FILTER_HPP
