/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PARTICLE_EDITOR_DOC_HPP
#define PARTICLE_EDITOR_DOC_HPP

class ParticleEditorDoc : public CDocument
{
protected:
    ParticleEditorDoc();

    DECLARE_DYNCREATE(ParticleEditorDoc)

public:
    /*virtual*/ ~ParticleEditorDoc();

    /*virtual*/ BOOL OnNewDocument();

    /*virtual*/ BOOL OnOpenDocument(LPCTSTR pathname);

    void SetActionProperty(int actionSelect);

    static ParticleEditorDoc &instance();

protected:
    DECLARE_MESSAGE_MAP()

private:
    static ParticleEditorDoc    *s_instance_;
    int                         m_actionSelect;
    bool                        m_check;
};

#endif // PARTICLE_EDITOR_DOC_HPP
