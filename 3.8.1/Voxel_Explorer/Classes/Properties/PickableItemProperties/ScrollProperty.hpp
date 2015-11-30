//
//  ScrollProperty.hpp
//  Voxel_Explorer
//
//  Created by wang haibo on 15/10/24.
//
//

#ifndef ScrollProperty_hpp
#define ScrollProperty_hpp

#include "PickableItemProperty.hpp"
class ScrollProperty : public PickableItemProperty, public IStackable
{
public:
    ScrollProperty(unsigned int instanceID, PickableItem::PickableItemType type);
    
    virtual void adjustByLevel();
    virtual void handleIdentify();
    
    virtual void increaseCount() { ++m_nCount; }
    virtual void decreaseCount() { --m_nCount; }
    
    virtual void addCount(CChaosNumber count) { m_nCount += count; }
    virtual void removeCount(CChaosNumber count) { m_nCount -= count; }

};

#endif /* ScrollProperty_hpp */
