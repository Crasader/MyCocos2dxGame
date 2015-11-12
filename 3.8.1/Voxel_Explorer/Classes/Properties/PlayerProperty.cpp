//
//  PlayerProperty.cpp
//  Voxel_Explorer
//
//  Created by wang haibo on 15/10/19.
//
//

#include "PlayerProperty.hpp"
#include "GameFormula.hpp"
#include "EventConst.h"
#include "KeyProperty.hpp"
#include "WeaponProperty.hpp"
#include "SecondWeaponProperty.hpp"
#include "ArmorProperty.hpp"
#include "MagicOrnamentProperty.hpp"
#include "ScrollProperty.hpp"
#include "PotionsProperty.hpp"
#include "MaterialProperty.hpp"
#include "QuestItemProperty.hpp"
#include "SundriesProperty.hpp"
USING_NS_CC;

unsigned int PlayerProperty::m_snItemInstanceIDCounter = 0;
PlayerProperty* g_pPlayerPropertyInstance = nullptr;
PlayerProperty* PlayerProperty::getInstance()
{
    if( g_pPlayerPropertyInstance == nullptr )
        g_pPlayerPropertyInstance = new PlayerProperty();
    
    return g_pPlayerPropertyInstance;
}
PlayerProperty::PlayerProperty()
{
    m_nGold                 = 0;                ///金币
    m_nSilver               = 0;                ///银币
    m_nCopper               = 0;                ///铜币
    m_nLevel                = 1;                ///等级
    m_nExp                  = 0;                ///经验
    m_nLightDistance        = 6;                ///光照范围
    m_nSearchDistance       = 1;                ///侦查范围
    m_nMaxHP                = 60;               ///最大生命值
    m_nMaxMP                = 60;               ///最大魔法值
    m_nCurrentHP            = 60;               ///当前生命值
    m_nCurrentMP            = 60;               ///当前魔法值
    m_nAddedMinAttack       = 1;                ///额外最小攻击增加值
    m_nAddedMaxAttack       = 4;                ///额外最大攻击增加值
    m_nAttackDiceNum        = 0;                ///攻击骰子数
    m_nAttackDiceFaceNum    = 0;                ///攻击骰子面数
    m_nArmorClass           = 4;                ///防御等级
    m_nBaseArmorClass       = 4;                ///基础防御等级
    m_fBlockRate            = 0.01f;            ///格挡率
    m_fCriticalStrikeRate   = 0.01f;            ///暴击率
    m_fDodgeRate            = 0.02f;            ///闪避率
    m_fBasicMagicItemFindRate = 0.2f;           ///基本魔法物品获得率
    m_fMagicItemFindRate    = m_fBasicMagicItemFindRate;             ///魔法物品获得率
    m_fMaxMagicItemFindRate = 0.8f;             ///最大魔法物品获得率
    
    m_nEquipedWeaponID      = -1;               ///装备了武器ID
    m_nEquipedSecondWeaponID= -1;               ///装备了副手武器ID
    m_nEquipedArmorID       = -1;               ///装备了护甲ID
    m_nEquipedOrnamentsID   = -1;               ///装备了饰品ID
    
    m_nBagMaxSpace          = 15;               ///背包最大容量
    m_nBagExtendTimes       = 0;                ///背包扩容次数
    m_nBagExtendMaxTimes    = 4;                ///背包最大扩容次数
    
    m_bDirty = false;
}
PlayerProperty::~PlayerProperty()
{
}
bool PlayerProperty::initNewPlayer()   ///新角色初始化
{
    bool ret = addItemToBag(PickableItem::PIT_DAGGER_DAGGER, 1);
    ret = equipWeapon(0);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORHEALTH, 1);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORHEALTH, 1);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORHEALTH, 1);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORMANA, 1);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORMANA, 1);
    ret = addItemToBag(PickableItem::PIT_POTION_MINORMANA, 1);
    ret = addItemToBag(PickableItem::PIT_SCROLL_INDENTIFY, 1);
    ret = addItemToBag(PickableItem::PIT_SCROLL_INDENTIFY, 1);
    ret = addItemToBag(PickableItem::PIT_SCROLL_INDENTIFY, 1);
    return ret;
}
void PlayerProperty::update(float delta)
{
    if(m_bDirty)
    {
        m_bDirty = false;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_PROPERTY_DIRTY);
    }
}
CChaosNumber PlayerProperty::getMinAttack()
{
    return m_nAttackDiceNum + m_nAddedMinAttack.GetLongValue();
}
CChaosNumber PlayerProperty::getMaxAttack()
{
    return m_nAttackDiceNum*m_nAttackDiceFaceNum.GetLongValue() + m_nAddedMaxAttack.GetLongValue();
}
CChaosNumber PlayerProperty::getDefense()
{
    return -m_nArmorClass.GetLongValue() + m_nBaseArmorClass.GetLongValue();
}
void PlayerProperty::addMoney(CChaosNumber gold, CChaosNumber silver, CChaosNumber copper)
{
    if(silver<0 || silver>99 || copper<0 || copper>99 )
    {
        CCLOGERROR("Money add error!");
        return;
    }
    m_nGold = m_nGold + gold.GetLongValue();
    m_nSilver = m_nSilver + silver.GetLongValue();
    m_nCopper = m_nCopper + copper.GetLongValue();
    m_bDirty = true;
}
void PlayerProperty::costMoney(CChaosNumber gold, CChaosNumber silver, CChaosNumber copper)
{
    CChaosNumber cost = GameFormula::exchangeMoney(gold,silver,copper);
    CChaosNumber own = GameFormula::exchangeMoney(m_nGold,m_nSilver,m_nCopper);
    if(own < cost)
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_MONEY_NOT_ENOUGH);
    else
    {
        CChaosNumber left = own.GetLongValue() - cost.GetLongValue();
        GameFormula::exchangeMoney(left, m_nGold, m_nSilver, m_nCopper);
    }
    m_bDirty = true;
}
void PlayerProperty::setExp(CChaosNumber exp)
{
    int needExp = GameFormula::getNextLevelExp(m_nLevel);
    if(exp >= needExp)
    {
        m_nExp = exp - needExp;
        levelUp();
    }
    else
        m_nExp = exp;
    m_bDirty = true;
}
void PlayerProperty::setCurrentHP(CChaosNumber hp)
{
    if(hp >= m_nMaxHP)
        m_nCurrentHP = m_nMaxHP;
    else if(hp <= 0)
    {
        m_nCurrentHP = 0;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_DEATH);
    }
    else
        m_nCurrentHP = hp;
    
    m_bDirty = true;
}
void PlayerProperty::setCurrentMP(CChaosNumber mp)
{
    if(mp >= m_nMaxMP)
        m_nMaxMP = mp;
    else if(mp <= 0)
    {
        m_nCurrentMP = 0;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_NO_MANA);
    }
    else
        m_nCurrentMP = mp;
    
    m_bDirty = true;
}
bool PlayerProperty::equipWeapon(CChaosNumber id)
{
    WeaponProperty* weaponProperty = static_cast<WeaponProperty*>(getItemFromBag(id));
    if(weaponProperty)
    {
        ///检查是否可装备
        if(weaponProperty->hasEquiped() || weaponProperty->getLevel() > m_nLevel)
            return false;
        ///卸载旧武器
        WeaponProperty* oldWeaponProperty = static_cast<WeaponProperty*>(getItemFromBag(m_nEquipedWeaponID));
        if(oldWeaponProperty)
        {
            ///检测旧装备是否可卸载
            if(!oldWeaponProperty->hasEquiped() || oldWeaponProperty->isCursed())
                return false;
            m_nLightDistance = m_nLightDistance - oldWeaponProperty->getAddedLightDistance().GetLongValue();
            m_nLightDistance = MAX(m_nLightDistance.GetLongValue(), 6);
            m_nSearchDistance = m_nSearchDistance - oldWeaponProperty->getAddedSearchDistance().GetLongValue();
            m_nSearchDistance = MAX(m_nSearchDistance.GetLongValue(), 1);
            m_nMaxHP = m_nMaxHP - oldWeaponProperty->getAddedMaxHp().GetLongValue();
            m_nMaxHP = MAX(m_nMaxHP.GetLongValue(), 30);
            m_nMaxMP = m_nMaxMP - oldWeaponProperty->getAddedMaxMp().GetLongValue();
            m_nMaxMP = MAX(m_nMaxMP.GetLongValue(), 30);
            m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
            m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);
            m_nAddedMinAttack = m_nAddedMinAttack - oldWeaponProperty->getAddedMinAttack().GetLongValue();
            m_nAddedMinAttack = MAX(0, m_nAddedMinAttack.GetLongValue());
            m_nAddedMaxAttack = m_nAddedMaxAttack - oldWeaponProperty->getAddedMaxAttack().GetLongValue();
            m_nAddedMaxAttack = MAX(0, m_nAddedMaxAttack.GetLongValue());
            m_nAttackDiceNum = 0;
            m_nAttackDiceFaceNum = 0;
            m_fCriticalStrikeRate = m_fCriticalStrikeRate - oldWeaponProperty->getAddedCriticalStrikeRate().GetFloatValue();
            m_fCriticalStrikeRate = MAX(0, m_fCriticalStrikeRate.GetFloatValue());
            m_fMagicItemFindRate = m_fMagicItemFindRate - m_fBasicMagicItemFindRate*weaponProperty->getAddedMagicItemFindRate().GetFloatValue();
            m_fMagicItemFindRate = MAX(0, m_fBasicMagicItemFindRate.GetFloatValue());
            oldWeaponProperty->setEquiped(false);
        }
        m_nEquipedWeaponID = id;
        
        ///装备新武器
        m_nLightDistance = m_nLightDistance + weaponProperty->getAddedLightDistance().GetLongValue();
        m_nSearchDistance = m_nSearchDistance + weaponProperty->getAddedSearchDistance().GetLongValue();
        m_nMaxHP = m_nMaxHP + weaponProperty->getAddedMaxHp().GetLongValue();
        m_nMaxMP = m_nMaxMP + weaponProperty->getAddedMaxMp().GetLongValue();
        m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
        m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);
        m_nAddedMinAttack = m_nAddedMinAttack + weaponProperty->getAddedMinAttack().GetLongValue();
        m_nAddedMaxAttack = m_nAddedMaxAttack + weaponProperty->getAddedMaxAttack().GetLongValue();
        m_nAttackDiceNum = weaponProperty->getAttackDiceNum().GetLongValue();
        m_nAttackDiceFaceNum = weaponProperty->getAttackDiceFaceNum().GetLongValue();
        m_fCriticalStrikeRate = m_fCriticalStrikeRate + weaponProperty->getAddedCriticalStrikeRate().GetFloatValue();
        m_fMagicItemFindRate = m_fMagicItemFindRate + m_fBasicMagicItemFindRate*weaponProperty->getAddedMagicItemFindRate().GetFloatValue();
        m_fMagicItemFindRate = MIN(m_fMagicItemFindRate, m_fMaxMagicItemFindRate);
        weaponProperty->setEquiped(true);
        m_bDirty = true;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_EQUIPED_WEAPON, &m_nEquipedWeaponID);
    }
    return true;
}
bool PlayerProperty::equipSecondWeapon(CChaosNumber id)
{
    SecondWeaponProperty* secondWeaponProperty = static_cast<SecondWeaponProperty*>(getItemFromBag(id));
    if(secondWeaponProperty)
    {
        ///检查是否可装备
        if(secondWeaponProperty->hasEquiped() || secondWeaponProperty->getLevel() > m_nLevel)
            return false;
        ///卸载旧武器
        SecondWeaponProperty* oldSecondWeaponProperty = static_cast<SecondWeaponProperty*>(getItemFromBag(m_nEquipedSecondWeaponID));
        if(oldSecondWeaponProperty)
        {
            ///检测旧装备是否可卸载
            if(!oldSecondWeaponProperty->hasEquiped() || oldSecondWeaponProperty->isCursed())
                return false;
            m_nLightDistance = m_nLightDistance - oldSecondWeaponProperty->getAddedLightDistance().GetLongValue();
            m_nLightDistance = MAX(m_nLightDistance.GetLongValue(), 6);
            m_nSearchDistance = m_nSearchDistance - oldSecondWeaponProperty->getAddedSearchDistance().GetLongValue();
            m_nSearchDistance = MAX(m_nSearchDistance.GetLongValue(), 1);
            m_nMaxHP = m_nMaxHP - oldSecondWeaponProperty->getAddedMaxHp().GetLongValue();
            m_nMaxHP = MAX(m_nMaxHP.GetLongValue(), 30);
            m_nMaxMP = m_nMaxMP - oldSecondWeaponProperty->getAddedMaxMp().GetLongValue();
            m_nMaxMP = MAX(m_nMaxMP.GetLongValue(), 30);
            m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
            m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);
            m_nAddedMinAttack = m_nAddedMinAttack - oldSecondWeaponProperty->getAddedMinAttack().GetLongValue();
            m_nAddedMinAttack = MAX(0, m_nAddedMinAttack.GetLongValue());
            m_nAddedMaxAttack = m_nAddedMaxAttack - oldSecondWeaponProperty->getAddedMaxAttack().GetLongValue();
            m_nAddedMaxAttack = MAX(0, m_nAddedMaxAttack.GetLongValue());
            
            m_nArmorClass = m_nArmorClass - oldSecondWeaponProperty->getAddedArmorClass().GetLongValue();
            m_nArmorClass = MIN(m_nBaseArmorClass.GetLongValue(), m_nArmorClass.GetLongValue());
            
            m_fBlockRate = m_fBlockRate - oldSecondWeaponProperty->getAddedBlockRate().GetFloatValue();
            m_fBlockRate = MAX(0, m_fBlockRate.GetFloatValue());
            m_fCriticalStrikeRate = m_fCriticalStrikeRate - oldSecondWeaponProperty->getAddedCriticalStrikeRate().GetFloatValue();
            m_fCriticalStrikeRate = MAX(0, m_fCriticalStrikeRate.GetFloatValue());
            m_fDodgeRate = m_fDodgeRate - oldSecondWeaponProperty->getAddedDodgeRate().GetFloatValue();
            m_fDodgeRate = MAX(0, m_fDodgeRate.GetFloatValue());
            m_fMagicItemFindRate = m_fMagicItemFindRate - m_fBasicMagicItemFindRate*oldSecondWeaponProperty->getAddedMagicItemFindRate().GetFloatValue();
            m_fMagicItemFindRate = MAX(0, m_fBasicMagicItemFindRate.GetFloatValue());
            oldSecondWeaponProperty->setEquiped(false);
        }
        m_nEquipedSecondWeaponID = id;
        
        ///装备新武器
        m_nLightDistance = m_nLightDistance + secondWeaponProperty->getAddedLightDistance().GetLongValue();
        m_nSearchDistance = m_nSearchDistance + secondWeaponProperty->getAddedSearchDistance().GetLongValue();
        m_nMaxHP = m_nMaxHP + secondWeaponProperty->getAddedMaxHp().GetLongValue();
        m_nMaxMP = m_nMaxMP + secondWeaponProperty->getAddedMaxMp().GetLongValue();
        m_nAddedMinAttack = m_nAddedMinAttack + secondWeaponProperty->getAddedMinAttack().GetLongValue();
        m_nAddedMaxAttack = m_nAddedMaxAttack + secondWeaponProperty->getAddedMaxAttack().GetLongValue();

        m_nArmorClass = m_nArmorClass + secondWeaponProperty->getAddedArmorClass().GetLongValue();
        
        m_fBlockRate = m_fBlockRate + secondWeaponProperty->getAddedBlockRate().GetFloatValue();
        m_fCriticalStrikeRate = m_fCriticalStrikeRate + secondWeaponProperty->getAddedCriticalStrikeRate().GetFloatValue();
        m_fDodgeRate = m_fDodgeRate + secondWeaponProperty->getAddedDodgeRate().GetFloatValue();
        
        m_fMagicItemFindRate = m_fMagicItemFindRate + m_fBasicMagicItemFindRate*secondWeaponProperty->getAddedMagicItemFindRate().GetFloatValue();
        m_fMagicItemFindRate = MIN(m_fMagicItemFindRate, m_fMaxMagicItemFindRate);
        secondWeaponProperty->setEquiped(true);
        
        m_bDirty = true;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_EQUIPED_WEAPON, &m_nEquipedWeaponID);
    }
    return true;
}
bool PlayerProperty::equipArmor(CChaosNumber id)
{
    ArmorProperty* armorProperty = static_cast<ArmorProperty*>(getItemFromBag(id));
    if(armorProperty)
    {
        ///检查是否可装备
        if(armorProperty->hasEquiped() || armorProperty->getLevel() > m_nLevel)
            return false;
        ///卸载旧护具
        ArmorProperty* oldArmorProperty = static_cast<ArmorProperty*>(getItemFromBag(m_nEquipedArmorID));
        if(oldArmorProperty)
        {
            ///检测旧装备是否可卸载
            if(!oldArmorProperty->hasEquiped() || oldArmorProperty->isCursed())
                return false;
            m_nLightDistance = m_nLightDistance - oldArmorProperty->getAddedLightDistance().GetLongValue();
            m_nLightDistance = MAX(m_nLightDistance.GetLongValue(), 6);
            m_nSearchDistance = m_nSearchDistance - oldArmorProperty->getAddedSearchDistance().GetLongValue();
            m_nSearchDistance = MAX(m_nSearchDistance.GetLongValue(), 1);
            m_nMaxHP = m_nMaxHP - oldArmorProperty->getAddedMaxHp().GetLongValue();
            m_nMaxHP = MAX(m_nMaxHP.GetLongValue(), 30);
            m_nMaxMP = m_nMaxMP - oldArmorProperty->getAddedMaxMp().GetLongValue();
            m_nMaxMP = MAX(m_nMaxMP.GetLongValue(), 30);
            m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
            m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);
            
            m_nArmorClass = m_nArmorClass - oldArmorProperty->getAddedArmorClass().GetLongValue();
            m_nArmorClass = MIN(m_nBaseArmorClass.GetLongValue(), m_nArmorClass.GetLongValue());
            
            m_fDodgeRate = m_fDodgeRate - oldArmorProperty->getAddedDodgeRate().GetFloatValue();
            m_fDodgeRate = MIN(0, m_fDodgeRate.GetFloatValue());
            
            m_fMagicItemFindRate = m_fMagicItemFindRate - m_fBasicMagicItemFindRate*oldArmorProperty->getAddedMagicItemFindRate().GetFloatValue();
            m_fMagicItemFindRate = MAX(0, m_fBasicMagicItemFindRate.GetFloatValue());
            oldArmorProperty->setEquiped(false);
        }
        
        m_nEquipedArmorID = id;
        
        ///装备新护具
        m_nLightDistance = m_nLightDistance + armorProperty->getAddedLightDistance().GetLongValue();
        m_nSearchDistance = m_nSearchDistance + armorProperty->getAddedSearchDistance().GetLongValue();
        m_nMaxHP = m_nMaxHP + armorProperty->getAddedMaxHp().GetLongValue();
        m_nMaxMP = m_nMaxMP + armorProperty->getAddedMaxMp().GetLongValue();
        m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
        m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);

        m_nArmorClass = m_nArmorClass + armorProperty->getAddedDodgeRate().GetFloatValue();
        m_fDodgeRate = m_fDodgeRate + armorProperty->getAddedDodgeRate().GetFloatValue();
        
        m_fMagicItemFindRate = m_fMagicItemFindRate + m_fBasicMagicItemFindRate*armorProperty->getAddedMagicItemFindRate().GetFloatValue();
        m_fMagicItemFindRate = MIN(m_fMagicItemFindRate, m_fMaxMagicItemFindRate);
        armorProperty->setEquiped(true);
        m_bDirty = true;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_EQUIPED_ARMOR, &m_nEquipedArmorID);
    }
    return true;
}
bool PlayerProperty::equipOrnaments(CChaosNumber id)
{
    MagicOrnamentProperty* magicOrnamentProperty = static_cast<MagicOrnamentProperty*>(getItemFromBag(id));
    if(magicOrnamentProperty)
    {
        ///检查是否可装备
        if(magicOrnamentProperty->hasEquiped() || magicOrnamentProperty->getLevel() > m_nLevel)
            return false;
        ///卸载旧饰品
        SecondWeaponProperty* oldMagicOrnamentProperty = static_cast<SecondWeaponProperty*>(getItemFromBag(m_nEquipedSecondWeaponID));
        if(oldMagicOrnamentProperty)
        {
            ///检测旧装备是否可卸载
            if(!oldMagicOrnamentProperty->hasEquiped() || oldMagicOrnamentProperty->isCursed())
                return false;
            m_nLightDistance = m_nLightDistance - oldMagicOrnamentProperty->getAddedLightDistance().GetLongValue();
            m_nLightDistance = MAX(m_nLightDistance.GetLongValue(), 6);
            m_nSearchDistance = m_nSearchDistance - oldMagicOrnamentProperty->getAddedSearchDistance().GetLongValue();
            m_nSearchDistance = MAX(m_nSearchDistance.GetLongValue(), 1);
            m_nMaxHP = m_nMaxHP - oldMagicOrnamentProperty->getAddedMaxHp().GetLongValue();
            m_nMaxHP = MAX(m_nMaxHP.GetLongValue(), 30);
            m_nMaxMP = m_nMaxMP - oldMagicOrnamentProperty->getAddedMaxMp().GetLongValue();
            m_nMaxMP = MAX(m_nMaxMP.GetLongValue(), 30);
            m_nCurrentHP = MIN(m_nCurrentHP, m_nMaxHP);
            m_nCurrentMP = MIN(m_nCurrentMP, m_nMaxMP);
            m_nAddedMinAttack = m_nAddedMinAttack - oldMagicOrnamentProperty->getAddedMinAttack().GetLongValue();
            m_nAddedMinAttack = MAX(0, m_nAddedMinAttack.GetLongValue());
            m_nAddedMaxAttack = m_nAddedMaxAttack - oldMagicOrnamentProperty->getAddedMaxAttack().GetLongValue();
            m_nAddedMaxAttack = MAX(0, m_nAddedMaxAttack.GetLongValue());
            
            m_nArmorClass = m_nArmorClass - oldMagicOrnamentProperty->getAddedArmorClass().GetLongValue();
            m_nArmorClass = MIN(m_nBaseArmorClass.GetLongValue(), m_nArmorClass.GetLongValue());
            
            m_fBlockRate = m_fBlockRate - oldMagicOrnamentProperty->getAddedBlockRate().GetFloatValue();
            m_fBlockRate = MAX(0, m_fBlockRate.GetFloatValue());
            m_fCriticalStrikeRate = m_fCriticalStrikeRate - oldMagicOrnamentProperty->getAddedCriticalStrikeRate().GetFloatValue();
            m_fCriticalStrikeRate = MAX(0, m_fCriticalStrikeRate.GetFloatValue());
            m_fDodgeRate = m_fDodgeRate - oldMagicOrnamentProperty->getAddedDodgeRate().GetFloatValue();
            m_fDodgeRate = MAX(0, m_fDodgeRate.GetFloatValue());
            m_fMagicItemFindRate = m_fMagicItemFindRate - m_fBasicMagicItemFindRate*oldMagicOrnamentProperty->getAddedMagicItemFindRate().GetFloatValue();
            m_fMagicItemFindRate = MAX(0, m_fBasicMagicItemFindRate.GetFloatValue());
            magicOrnamentProperty->setEquiped(false);
        }
        m_nEquipedOrnamentsID = id;
        
        ///装备新饰品
        m_nLightDistance = m_nLightDistance + magicOrnamentProperty->getAddedLightDistance().GetLongValue();
        m_nSearchDistance = m_nSearchDistance + magicOrnamentProperty->getAddedSearchDistance().GetLongValue();
        m_nMaxHP = m_nMaxHP + magicOrnamentProperty->getAddedMaxHp().GetLongValue();
        m_nMaxMP = m_nMaxMP + magicOrnamentProperty->getAddedMaxMp().GetLongValue();
        m_nAddedMinAttack = m_nAddedMinAttack + magicOrnamentProperty->getAddedMinAttack().GetLongValue();
        m_nAddedMaxAttack = m_nAddedMaxAttack + magicOrnamentProperty->getAddedMaxAttack().GetLongValue();
        
        m_nArmorClass = m_nArmorClass + magicOrnamentProperty->getAddedArmorClass().GetLongValue();
        
        m_fBlockRate = m_fBlockRate + magicOrnamentProperty->getAddedBlockRate().GetFloatValue();
        m_fCriticalStrikeRate = m_fCriticalStrikeRate + magicOrnamentProperty->getAddedCriticalStrikeRate().GetFloatValue();
        m_fDodgeRate = m_fDodgeRate + magicOrnamentProperty->getAddedDodgeRate().GetFloatValue();
        
        m_fMagicItemFindRate = m_fMagicItemFindRate + m_fBasicMagicItemFindRate*magicOrnamentProperty->getAddedMagicItemFindRate().GetFloatValue();
        m_fMagicItemFindRate = MIN(m_fMagicItemFindRate, m_fMaxMagicItemFindRate);
        magicOrnamentProperty->setEquiped(true);
        m_bDirty = true;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_EQUIPED_ORNAMENTS, &m_nEquipedOrnamentsID);
    }
    return true;
}
bool PlayerProperty::indentifyItem(CChaosNumber id)
{
    PickableItemProperty* pickableItemProperty = static_cast<PickableItemProperty*>(getItemFromBag(id));
    if(pickableItemProperty && !pickableItemProperty->isIdentified())
    {
        pickableItemProperty->handleIdentify();
        return true;
    }
    return false;
}
bool PlayerProperty::usePotion(CChaosNumber id)
{
    PotionsProperty* potionsProperty = static_cast<PotionsProperty*>(getItemFromBag(id));
    if(potionsProperty && potionsProperty->getCount() >= 1)
    {
        return true;
    }
    return false;
}
bool PlayerProperty::useScroll(CChaosNumber id)
{
    ScrollProperty* scrollProperty = static_cast<ScrollProperty*>(getItemFromBag(id));
    if(scrollProperty && scrollProperty->getCount() >= 1)
    {
        return true;
    }
    return false;
}
bool PlayerProperty::useKey(PickableItem::PickableItemType type)
{
    return false;
}

bool PlayerProperty::addItemToBag(PickableItem::PickableItemType type, CChaosNumber level)
{
    if(m_Bag.size() >= m_nBagMaxSpace.GetLongValue())
    {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_BAG_NO_SPACE);
        return false;
    }
    for (PickableItemProperty* item : m_Bag) {
        if(item)
        {
            if(item->isStackable() && item->getPickableItemType() == type)
            {
                IStackable* itemProperty = dynamic_cast<IStackable*>(item);
                if (itemProperty)
                    itemProperty->increaseCount();
                if(item->isIdentified())
                    item->adjustByLevel();
                return true;
            }
        }
    }
    PickableItemProperty* itemProperty = nullptr;
    if(type >= PickableItem::PIT_KEY_COPPER && type <= PickableItem::PIT_KEY_ROOM)
        itemProperty = new (std::nothrow) KeyProperty(m_snItemInstanceIDCounter++,type);
    else if (type >= PickableItem::PIT_DAGGER_DAGGER && type <= PickableItem::PIT_MACE_PRO_SLEDGEHAMMER)
        itemProperty = new (std::nothrow) WeaponProperty(m_snItemInstanceIDCounter++,type, level, !GameFormula::generateMagicItem(m_fMagicItemFindRate.GetFloatValue()));
    else if (type >= PickableItem::PIT_BOW_SHORTBOW && type <=PickableItem::PIT_SHIELD_PRO_TOWERSHIELD)
        itemProperty = new (std::nothrow) SecondWeaponProperty(m_snItemInstanceIDCounter++,type, level, !GameFormula::generateMagicItem(m_fMagicItemFindRate.GetFloatValue()));
    else if(type >= PickableItem::PIT_CLOTH_SHOES && type <= PickableItem::PIT_CLOTH_PRO_STEELARMOR)
        itemProperty = new (std::nothrow) ArmorProperty(m_snItemInstanceIDCounter++,type, level, !GameFormula::generateMagicItem(m_fMagicItemFindRate.GetFloatValue()));
    else if(type >= PickableItem::PIT_SCROLL_INDENTIFY && type <= PickableItem::PIT_SCROLL_DESTINY)
        itemProperty = new (std::nothrow) ScrollProperty(m_snItemInstanceIDCounter++,type);
    else if(type >= PickableItem::PIT_POTION_MINORHEALTH && type <= PickableItem::PIT_POTION_SPECIFIC)
        itemProperty = new (std::nothrow) PotionsProperty(m_snItemInstanceIDCounter++,type);
    if(itemProperty)
    {
        if(itemProperty->isIdentified())
            itemProperty->adjustByLevel();
        m_Bag.push_back(itemProperty);
        return true;
    }
    return false;
}
bool PlayerProperty::removeStackableItemFromBag(CChaosNumber id, CChaosNumber count)
{
    std::vector<PickableItemProperty*>::iterator iter;
    for (iter = m_Bag.begin(); iter != m_Bag.end(); iter++) {
        if((*iter) != nullptr && (*iter)->getInstanceID() == id.GetLongValue())
        {
            if((*iter)->getCount() == count)
            {
                m_Bag.erase(iter);
                return true;
            }
            else if((*iter)->getCount() > count)
            {
                (*iter)->setCount((*iter)->getCount().GetLongValue() - count.GetLongValue());
                return true;
            }
            else
                return false;
        }
    }
    return false;
}
bool PlayerProperty::removeItemFromBag(CChaosNumber id)
{
    std::vector<PickableItemProperty*>::iterator iter;
    for (iter = m_Bag.begin(); iter != m_Bag.end(); iter++) {
        if((*iter) != nullptr && (*iter)->getInstanceID() == id.GetLongValue())
        {
            m_Bag.erase(iter);
            return true;
        }
    }
    return false;
}
void PlayerProperty::extendBagSpace()
{
    if(m_nBagExtendTimes >= m_nBagExtendMaxTimes)
    {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_BAG_EXTEND_HAS_REACH_MAXTIMES);
    }
    else
    {
        m_nBagExtendTimes = m_nBagExtendTimes + 1;
        m_nBagMaxSpace = m_nBagExtendTimes*15;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_BAG_EXTEND_OK);
    }
}
CChaosNumber PlayerProperty::getRandomAttack()
{
    return cocos2d::random(getMinAttack().GetLongValue(), getMaxAttack().GetLongValue());
}
void PlayerProperty::load()
{
}
void PlayerProperty::save()
{
}
void PlayerProperty::levelUp()
{
    m_nLevel = m_nLevel + 1;
    m_nMaxHP = m_nMaxHP + 8;
    m_nMaxHP = m_nMaxMP + 8;
    m_nCurrentHP = m_nMaxHP;
    m_nCurrentMP = m_nMaxMP;
    m_bDirty = true;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_PLAYER_LEVEL_UP);
}
PickableItemProperty* PlayerProperty::getItemFromBag(CChaosNumber id) const
{
    std::vector<PickableItemProperty*>::const_iterator iter;
    for (iter = m_Bag.begin(); iter != m_Bag.end(); iter++) {
        if((*iter) != nullptr && (*iter)->getInstanceID() == id.GetLongValue())
            return (*iter);
    }
    return nullptr;
}