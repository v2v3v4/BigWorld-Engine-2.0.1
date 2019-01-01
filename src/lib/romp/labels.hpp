/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LABELS_HPP
#define LABELS_HPP

#include "romp/font.hpp"
#include "romp/font_metrics.hpp"
#include "romp/font_manager.hpp"
#include "moo/visual_channels.hpp"

/** This class allows labels to easily be drawn on the screen.
  * Just add the labels and their world positions to the list.
  * then call Moo::SortedChannel::addDrawItem( ChannelDrawItem labels )
  * and it will take care of the rest.
  */

class Labels : public Moo::ChannelDrawItem
{
public:
	struct Label
	{
		Label( const std::string& t, const Vector3& p )
			: text_(t), position_(p)
		{}

		std::string text_;
		Vector3 position_;
	};

public:
	Labels()
	{
		this->distance_ = 0;
	}

	virtual void draw()
	{
		Matrix viewProj = Moo::rc().view();
		viewProj.postMultiply( Moo::rc().projection() );
				
		FontPtr font = FontManager::instance().get( "system_small.font" );
		if ( font && FontManager::instance().begin( *font ) )
		{
			LabelVector::const_iterator it  = this->labels_.begin();
			LabelVector::const_iterator end = this->labels_.end();
			while (it != end)
			{
				Vector3 projectedPos = viewProj.applyPoint( it->position_ );
				if ( projectedPos.z <= 1.f )
				{
					font->drawStringInClip( it->text_, projectedPos );
				}

				++it;
			}
			FontManager::instance().end();
		}
	}

	virtual void fini()
	{
		delete this;
	}

	void add( const std::string & id, const Vector3 & position )
	{
		this->labels_.push_back( Label(id, position) );
	}

	typedef std::vector<Label> LabelVector;
	LabelVector labels_;
};

/** This class allows set of labels to easily be drawn on the screen.
  * This class differs to Labels in that it stacks all the labels on
  *	top of each other vertically rooted at a single position.
  */
class StackedLabels : public Moo::ChannelDrawItem
{
public:
	StackedLabels( const Vector3& position )
		:	position_(position)
	{
		this->distance_ = 0;
	}

	virtual void draw()
	{
		Matrix viewProj = Moo::rc().view();
		viewProj.postMultiply( Moo::rc().projection() );
		Vector3 projectedPos = viewProj.applyPoint( position_ );
		if ( projectedPos.z <= 1.f )
		{
			FontPtr font = FontManager::instance().get( "system_small.font" );
			if (FontManager::instance().begin( *font ))
			{
				float clipHeight = font->metrics().clipHeight();
				Vector3 pos = position_;

				StrVector::const_iterator it  = this->labels_.begin();
				StrVector::const_iterator end = this->labels_.end();
				while (it != end)
				{
					float widthInClip = font->metrics().stringWidth( *it ) / float(Moo::rc().halfScreenWidth());

					projectedPos.y += clipHeight;
					font->drawStringInClip( *it, projectedPos-Vector3(widthInClip/2.f,0,0) );
					++it;
				}
				FontManager::instance().end();
			}
		}
	}

	virtual void fini()
	{
		delete this;
	}

	void add( const std::string & id )
	{
		this->labels_.push_back( id );
	}

	typedef std::vector<std::string> StrVector;
	StrVector labels_;
	Vector3 position_;
};

#endif // LABELS_HPP
