#ifndef CGUIBANNER_H
#define CGUIBANNER_H

#include "CGUIText.h"

class CGUIBanner : public CGUIText
{
public:
    CGUIBanner(const std::string& text);
    
    void processLogic();
    void processRender(const CRect<float> &RectDispCoordFloat);
    void setText(const std::string& text);
    
private:    
    std::list<std::string>::iterator curTextIt;
    std::list<std::string>::iterator prevTextIt;
    bool transition;
    int timer;
    uint8_t alpha;
};

#endif // CGUIBANNER_H
