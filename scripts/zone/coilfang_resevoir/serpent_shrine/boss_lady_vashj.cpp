/* Copyright (C) 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111-1307USA
*/

/* ScriptData
SDName: Boss_Lady_Vashj
SD%Complete: 99
SDComment: Missing blizzlike Shield Generators coords
EndScriptData */

#include "../../../sc_defines.h"
#include "../../../creature/simple_ai.h"
#include "../../../../../../game/GameObject.h"
#include "../../../../../../game/Player.h"
#include "../../../../../../game/Spell.h"
#include "../../../../../../game/Item.h"
#include "../../../../../../game/TargetedMovementGenerator.h"

#define SPELL_MULTI_SHOT              38310
#define SPELL_SHOCK_BLAST             38509
#define SPELL_ENTANGLE                38316
#define SPELL_STATIC_CHARGE_TRIGGER   38280
#define SPELL_FORKED_LIGHTNING        40088
#define SPELL_SHOOT                   40873
#define SPELL_POISON_BOLT             40095
#define SPELL_TOXIC_SPORES            38575
#define SPELL_MAGIC_BARRIER           38112

#define SAY_INTRO                     "Water is life. It has become a rare commodity here in Outland. A commodity that we alone shall control. We are the Highborne, and the time has come at last for us to retake our rightful place in the world!"
#define SAY_AGGRO1                    "I'll split you from stem to stern! "
#define SAY_AGGRO2                    "Victory to Lord Illidan!"
#define SAY_AGGRO3                    "I spit on you, surface filth!"
#define SAY_AGGRO4                    "Death to the outsiders!"
#define SAY_PHASE1                    "I did not wish to lower myself by engaging your kind, but you leave me little choice!"
#define SAY_PHASE2                    "The time is now! Leave none standing!"
#define SAY_PHASE3                    "You may want to take cover."
#define SAY_BOWSHOT1                  "Straight to the heart!"
#define SAY_BOWSHOT2                  "Seek your mark!"
#define SAY_SLAY1                     "Your time ends now!"
#define SAY_SLAY2                     "You have failed!"
#define SAY_DEATH                     "Lord Illidan, I... I am... sorry."

#define SOUND_INTRO                   11531
#define SOUND_AGGRO1                  11532
#define SOUND_AGGRO2                  11533
#define SOUND_AGGRO3                  11534
#define SOUND_AGGRO4                  11535
#define SOUND_PHASE1                  11538
#define SOUND_PHASE2                  11539
#define SOUND_PHASE3                  11540
#define SOUND_BOWSHOT1                11536
#define SOUND_BOWSHOT2                11537
#define SOUND_SLAY1                   11541
#define SOUND_SLAY2                   11542
#define SOUND_DEATH                   11544

#define MIDDLE_X                      30.134
#define MIDDLE_Y                      -923.65
#define MIDDLE_Z                      42.9

#define SPOREBAT_X                    30.977156
#define SPOREBAT_Y                    -925.297761
#define SPOREBAT_Z                    77.176567
#define SPOREBAT_O                    5.223932

#define SHIED_GENERATOR_CHANNEL       19870
#define ENCHANTED_ELEMENTAL           21958
#define TAINTED_ELEMENTAL             22009
#define COILFANG_STRIDER              22056
#define COILFANG_ELITE                22055
#define FATHOM_SPOREBAT               22120 

float ElementPos[8][4] = 
{
    {8.3, -835.3, 21.9, 5},
    {53.4, -835.3, 21.9, 4.5},
    {96, -861.9, 21.8, 4},
    {96, -986.4, 21.4, 2.5},
    {54.4, -1010.6, 22, 1.8},
    {9.8, -1012, 21.7, 1.4},
    {-35, -987.6, 21.5, 0.8},
    {-58.9, -901.6, 21.5, 6}
};

float CoilfangElitePos[3][4] =
{
    {28.84, -923.28, 42.9, 6},
    {31.183281, -953.502625, 41.523602, 1.640957},
    {58.895180, -923.124268, 41.545307, 3.152848}
};

float CoilfangStriderPos[3][4] =
{
    {66.427010, -948.778503, 41.262245, 2.584220},
    {7.513962, -959.538208, 41.300422, 1.034629},
    {-12.843201, -907.798401, 41.239620, 6.087094}
};

float ShieldGeneratorChannelPos[4][4] =
{
    {49.6262, -902.181, 43.0975, 3.95683},
    {10.988, -901.616, 42.5371, 5.4373},
    {10.3859, -944.036, 42.5446, 0.779888},
    {49.3126, -943.398, 42.5501, 2.40174}
};

//Lady Vashj AI
struct MANGOS_DLL_DECL boss_lady_vashjAI : public ScriptedAI
{
    boss_lady_vashjAI (Creature *c) : ScriptedAI(c) 
    {
        pInstance = (c->GetInstanceData()) ? ((ScriptedInstance*)c->GetInstanceData()) : NULL;
        EnterEvadeMode();
    }

    ScriptedInstance *pInstance;

    uint64 ShieldGeneratorChannel[3];

    uint32 ShockBlast_Timer;
    uint32 Entangle_Timer;
    uint32 StaticCharge_Timer;
    uint32 ForkedLightning_Timer;
    uint32 Check_Timer;
    uint32 EnchantedElemental_Timer;
    uint32 TaintedElemental_Timer;
    uint32 CoilfangElite_Timer;
    uint32 CoilfangStrider_Timer;
    uint32 SummonSporebat_Timer;
    uint32 SummonSporebat_StaticTimer;
    uint8 EnchantedElemental_Pos;
    uint8 Phase;

    bool Entangle;
    bool InCombat;

    void EnterEvadeMode()
    {
        ShockBlast_Timer = 1+rand()%60000;
        Entangle_Timer = 30000;
        StaticCharge_Timer = 10000+rand()%15000;
        ForkedLightning_Timer = 2000;
        Check_Timer = 1000;
        EnchantedElemental_Timer = 5000;
        TaintedElemental_Timer = 50000;
        CoilfangElite_Timer = 45000+rand()%5000;
        CoilfangStrider_Timer = 60000+rand()%10000;
        SummonSporebat_Timer = 10000;
        SummonSporebat_StaticTimer = 30000;
        EnchantedElemental_Pos = 0;
        Phase = 0;

        Entangle = false;
        InCombat = false;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();

        if(pInstance)
            pInstance->SetData("LadyVashjEvent", 0);

        ShieldGeneratorChannel[0] = 0;
        ShieldGeneratorChannel[1] = 0;
        ShieldGeneratorChannel[2] = 0;
        ShieldGeneratorChannel[3] = 0;

        m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISARM, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, IMMUNE_SCHOOL_NATURE, true);
    }

    //Called when a tainted elemental dies
    void EventTaintedElementalDeath()
    {
        //the next will spawn 50 seconds after the previous one's death
        if(TaintedElemental_Timer > 50000)
            TaintedElemental_Timer = 50000;
    }

    void KilledUnit(Unit *victim)
    {
        switch(rand()%2)
        {
        case 0:
            DoYell(SAY_SLAY1, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(m_creature, SOUND_SLAY1);
            break;

        case 1:
            DoPlaySoundToSet(m_creature, SOUND_SLAY1);
            DoYell(SAY_SLAY2, LANG_UNIVERSAL, NULL);
            break;
        }
    }

    void JustDied(Unit *victim)
    {
        DoPlaySoundToSet(m_creature, SOUND_DEATH);
        DoYell(SAY_DEATH, LANG_UNIVERSAL, NULL);

        if(pInstance)
            pInstance->SetData("LadyVashjEvent", 0);
    }

    void StartEvent()
    {
        switch(rand()%4)
        {
        case 0:
            DoPlaySoundToSet(m_creature, SOUND_AGGRO1);
            DoYell(SAY_AGGRO1, LANG_UNIVERSAL, NULL);
            break;

        case 1:
            DoPlaySoundToSet(m_creature, SOUND_AGGRO2);
            DoYell(SAY_AGGRO2, LANG_UNIVERSAL, NULL);
            break;

        case 2:
            DoPlaySoundToSet(m_creature, SOUND_AGGRO3);
            DoYell(SAY_AGGRO3, LANG_UNIVERSAL, NULL);
            break;

        case 3:
            DoPlaySoundToSet(m_creature, SOUND_AGGRO4);
            DoYell(SAY_AGGRO4, LANG_UNIVERSAL, NULL);
            break;
        }

        InCombat = true;
        Phase = 1;

        if(pInstance)
            pInstance->SetData("LadyVashjEvent", 1);
    }

    void AttackStart(Unit *who)
    {
        if(!who && who != m_creature)
        return;

        if (who->isTargetableForAttack() && who!= m_creature)
        {
            //Begin melee attack if we are within range
            if(Phase != 2)
                DoStartMeleeAttack(who);

            if(!InCombat)
                StartEvent();
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || m_creature->getVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessablePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {
            float attackRadius = m_creature->GetAttackDistance(who);
            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
            {
                if(who->HasStealthAura())
                who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

                if(Phase != 2)
                    DoStartMeleeAttack(who);

                if(!InCombat)
                    StartEvent();
            }
        }
    }

    void CastShootOrMultishot()
    {
        switch(rand()%2)
        {
            case 0:
            //Shoot
            //Used in Phases 1 and 3 after Entangle or while having nobody in melee range. A shot that hits her target for 4097-5543 Physical damage.
            DoCast(m_creature->getVictim(), SPELL_SHOOT);
            break;

            case 1:
            //Multishot
            //Used in Phases 1 and 3 after Entangle or while having nobody in melee range. A shot that hits 1 person and 4 people around him for 6475-7525 physical damage.
            DoCast(m_creature->getVictim(), SPELL_MULTI_SHOT);
            break;
        }

        if(rand()%3)
        {
            switch(rand()%2)
            {
                case 0:
                DoPlaySoundToSet(m_creature, SOUND_BOWSHOT1);
                DoYell(SAY_BOWSHOT1, LANG_UNIVERSAL, NULL);
                break;

                case 1:
                DoPlaySoundToSet(m_creature, SOUND_BOWSHOT2);
                DoYell(SAY_BOWSHOT2, LANG_UNIVERSAL, NULL);
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //to prevent abuses during phase 2
        if(Phase == 2 && !m_creature->getVictim() && InCombat)
            EnterEvadeMode();

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        if(Phase == 1 || Phase == 3)
        {
            //ShockBlast_Timer
            if (ShockBlast_Timer < diff)
            { 
                //Shock Burst
                //Randomly used in Phases 1 and 3 on Vashj's target, it's a Shock spell doing 8325-9675 nature damage and stunning the target for 5 seconds, during which she will not attack her target but switch to the next person on the aggro list.
                DoCast(m_creature->getVictim(), SPELL_SHOCK_BLAST);
                m_creature->TauntApply(m_creature->getVictim());

                ShockBlast_Timer = 1000+rand()%14000; //random cooldown
            }else ShockBlast_Timer -= diff;

            //StaticCharge_Timer
            if(StaticCharge_Timer < diff)
            {
                //Static Charge
                //Used on random people (only 1 person at any given time) in Phases 1 and 3, it's a debuff doing 2775 to 3225 Nature damage to the target and everybody in about 5 yards around it, every 1 seconds for 30 seconds. It can be removed by Cloak of Shadows, Iceblock, Divine Shield, etc, but not by Cleanse or Dispel Magic.
                Unit *target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);

                if(target && !target->HasAura(SPELL_STATIC_CHARGE_TRIGGER, 0))
                    DoCast(target, SPELL_STATIC_CHARGE_TRIGGER); //cast Static Charge every 2 seconds for 20 seconds

                StaticCharge_Timer = 10000+rand()%20000; //blizzlike
            }else StaticCharge_Timer -= diff;

            //Entangle_Timer
            if (Entangle_Timer < diff)
            {
                if(!Entangle)
                {
                    //Entangle
                    //Used in Phases 1 and 3, it casts Entangling Roots on everybody in a 15 yard radius of Vashj, immobilzing them for 10 seconds and dealing 500 damage every 2 seconds. It's not a magic effect so it cannot be dispelled, but is removed by various buffs such as Cloak of Shadows or Blessing of Freedom.
                    DoCast(m_creature->getVictim(), SPELL_ENTANGLE);
                    Entangle = true;
                    Entangle_Timer = 10000;
                }
                else
                {
                    CastShootOrMultishot();
                    Entangle = false;
                    Entangle_Timer = 20000+rand()%5000;
                }
            }else Entangle_Timer -= diff;

            //Phase 1
            if(Phase == 1)
            {
                //Start phase 2
                if ((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 70)
                {
                    //Phase 2 begins when Vashj hits 70%. She will run to the middle of her platform and surround herself in a shield making her invulerable.
                    Phase = 2;

                    m_creature->GetMotionMaster()->Clear();
                    m_creature->Relocate(MIDDLE_X, MIDDLE_Y, MIDDLE_Z);
                    m_creature->SendMoveToPacket(MIDDLE_X, MIDDLE_Y, MIDDLE_Z, false, 0);

                    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    m_creature->RemoveAllAuras();

                    Creature *pCreature;
                    for(uint8 i = 0; i < 4; i++)
                    {
                        pCreature = m_creature->SummonCreature(SHIED_GENERATOR_CHANNEL, ShieldGeneratorChannelPos[i][0],  ShieldGeneratorChannelPos[i][1],  ShieldGeneratorChannelPos[i][2],  ShieldGeneratorChannelPos[i][3], TEMPSUMMON_CORPSE_DESPAWN, 0);
                        if (pCreature)
                            ShieldGeneratorChannel[i] = pCreature->GetGUID();
                    }

                    DoPlaySoundToSet(m_creature, SOUND_PHASE2);
                    DoYell(SAY_PHASE2, LANG_UNIVERSAL, NULL);
                }
            }
            //Phase 3
            else
            {
                //SummonSporebat_Timer
                if(SummonSporebat_Timer < diff)
                {
                    Creature *Sporebat = NULL;
                    Sporebat = m_creature->SummonCreature(FATHOM_SPOREBAT, SPOREBAT_X, SPOREBAT_Y, SPOREBAT_Z, SPOREBAT_O, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);

                    if(Sporebat)
                    {
                        Unit *target = NULL;
                        target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                        if(target)
                            Sporebat->AI()->AttackStart(target);
                    }

                    //summon sporebats faster and faster
                    if(SummonSporebat_StaticTimer > 1000)
                        SummonSporebat_StaticTimer -= 1000;

                    SummonSporebat_Timer = SummonSporebat_StaticTimer;
                }else SummonSporebat_Timer -= diff;
            }

            //Melee attack
            DoMeleeAttackIfReady();

            //Check_Timer - used to check if somebody is in melee range
            if(Check_Timer < diff)
            {
                bool InMeleeRange = false;
                Unit *target;
                std::list<HostilReference *> t_list = m_creature->getThreatManager().getThreatList();
                for(std::list<HostilReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
                {
                    target = Unit::GetUnit(*m_creature, (*itr)->getUnitGuid());
                    if(target && target->IsWithinDistInMap(m_creature, 5)) //if in melee range
                    {
                        InMeleeRange = true;
                        break;
                    }
                }

                //if nobody is in melee range
                if(!InMeleeRange)
                    CastShootOrMultishot();

                Check_Timer = 1000;
            }else Check_Timer -= diff;
        }
        //Phase 2
        else
        {
            //ForkedLightning_Timer
            if(ForkedLightning_Timer < diff)
            {
                //Forked Lightning
                //Used constantly in Phase 2, it shoots out completely randomly targeted bolts of lightning which hit everybody in a roughtly 60 degree cone in front of Vashj for 2313-2687 nature damage.
                Unit *target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);

                if(!target)
                    target = m_creature->getVictim();

                DoCast(target, SPELL_FORKED_LIGHTNING);

                ForkedLightning_Timer = 2000+rand()%6000; //blizzlike
            }else ForkedLightning_Timer -= diff;

            //EnchantedElemental_Timer
            if(EnchantedElemental_Timer < diff)
            {
                Creature *Elemental;
                Elemental = m_creature->SummonCreature(ENCHANTED_ELEMENTAL, ElementPos[EnchantedElemental_Pos][0], ElementPos[EnchantedElemental_Pos][1], ElementPos[EnchantedElemental_Pos][2], ElementPos[EnchantedElemental_Pos][3], TEMPSUMMON_CORPSE_DESPAWN, 0);

                if(EnchantedElemental_Pos == 7)
                    EnchantedElemental_Pos = 0;
                else
                    EnchantedElemental_Pos++;

                EnchantedElemental_Timer = 10000+rand()%5000;
            }else EnchantedElemental_Timer -= diff;

            //TaintedElemental_Timer
            if(TaintedElemental_Timer < diff)
            {
                Creature *Tain_Elemental;
                uint32 pos = rand()%8;
                Tain_Elemental = m_creature->SummonCreature(TAINTED_ELEMENTAL, ElementPos[pos][0], ElementPos[pos][1], ElementPos[pos][2], ElementPos[pos][3], TEMPSUMMON_DEAD_DESPAWN, 0);

                TaintedElemental_Timer = 120000;
            }else TaintedElemental_Timer -= diff;

            //CoilfangElite_Timer
            if(CoilfangElite_Timer < diff)
            {
                Creature *CoilfangElite;
                uint32 pos = rand()%3;
                CoilfangElite = m_creature->SummonCreature(COILFANG_ELITE, CoilfangElitePos[pos][0], CoilfangElitePos[pos][1], CoilfangElitePos[pos][2], CoilfangElitePos[pos][3], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                if(CoilfangElite)
                {
                    Unit *target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    if(target)
                        CoilfangElite->AI()->AttackStart(target);
                }

                CoilfangElite_Timer = 45000+rand()%5000; //wowwiki says 50 seconds, bosskillers says 45
            }else CoilfangElite_Timer -= diff;

            //CoilfangStrider_Timer
            if(CoilfangStrider_Timer < diff)
            {
                Creature *CoilfangStrider;
                uint32 pos = rand()%3;
                CoilfangStrider = m_creature->SummonCreature(COILFANG_STRIDER, CoilfangStriderPos[pos][0], CoilfangStriderPos[pos][1], CoilfangStriderPos[pos][2], CoilfangStriderPos[pos][3], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                if(CoilfangStrider)
                {
                    Unit *target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    if(target)
                        CoilfangStrider->AI()->AttackStart(target);
                }

                CoilfangStrider_Timer = 60000+rand()%10000; //wowwiki says 60 seconds, bosskillers says 60-70
            }else CoilfangStrider_Timer -= diff;

            //Check_Timer
            if(Check_Timer < diff)
            {
                //Start Phase 3
                if(pInstance && pInstance->GetData("CanStartPhase3"))
                {
                    //set life 50%
                    m_creature->SetHealth(m_creature->GetMaxHealth()/2);

                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    DoPlaySoundToSet(m_creature, SOUND_PHASE3);
                    DoYell(SAY_PHASE3, LANG_UNIVERSAL, NULL);

                    Phase = 3;

                    //return to the tank
                    m_creature->GetMotionMaster()->Mutate(new TargetedMovementGenerator<Creature>(*m_creature->getVictim()));
                }
                Check_Timer = 1000;
            }else Check_Timer -= diff;
        }
    }
};

//Enchanted Elemental
//If one of them reaches Vashj he will increase her damage done by 5%.
struct MANGOS_DLL_DECL mob_enchanted_elementalAI : public ScriptedAI
{
    mob_enchanted_elementalAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData()) ? ((ScriptedInstance*)c->GetInstanceData()) : NULL;
        EnterEvadeMode();
    }

    ScriptedInstance *pInstance;

    uint32 Check_Timer;
    uint32 Movement_Timer;

    void EnterEvadeMode()
    {
        Check_Timer = 5000;
        Movement_Timer = 500;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }

    void AttackStart(Unit *who) { return; }

    void MoveInLineOfSight(Unit *who) { return; }

    void UpdateAI(const uint32 diff)
    {
        //Movement_Timer
        if(Movement_Timer < diff)
        {
            Unit *Vashj = NULL;
            Vashj = Unit::GetUnit((*m_creature), pInstance->GetData64("LadyVashj"));
            if(Vashj)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->Mutate(new TargetedMovementGenerator<Creature>(*Vashj));
            }

            //if first movement doesn't work, apply the same movement after 10 seconds
            Movement_Timer = 5000;
        }else Movement_Timer -= diff;

        //Check_Timer
        if(Check_Timer < diff)
        {
            if(pInstance)
            {
                Unit *Vashj = NULL;
                Vashj = Unit::GetUnit((*m_creature), pInstance->GetData64("LadyVashj"));
                if(Vashj)
                {
                    if(Vashj->IsWithinDistInMap(m_creature, 5))
                    {
                        //increase lady vashj damage (+5%)
                        const CreatureInfo *cinfo = m_creature->GetCreatureInfo();
                        Vashj->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 5)));
                        Vashj->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 5)));
                        m_creature->UpdateDamagePhysical(BASE_ATTACK);

                        //call Unsummon()
                        m_creature->setDeathState(JUST_DIED);
                    }
                    else if(((boss_lady_vashjAI*)((Creature*)Vashj)->AI())->InCombat == false)
                    {
                        //call Unsummon()
                        m_creature->setDeathState(JUST_DIED);
                    }
                }
            }
            Check_Timer = 1000;
        }else Check_Timer -= diff;
    }
};

//Tainted Elemental
//This mob has 7,900 life, doesn't move, and shoots Poison Bolts at one person anywhere in the area, doing 3,000 nature damage and placing a posion doing 2,000 damage every 2 seconds. He will switch targets often, or sometimes just hang on a single player, but there is nothing you can do about it except heal the damage and kill the Tainted Elemental
struct MANGOS_DLL_DECL mob_tainted_elementalAI : public ScriptedAI
{
    mob_tainted_elementalAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData()) ? ((ScriptedInstance*)c->GetInstanceData()) : NULL;
        EnterEvadeMode();
    }

    ScriptedInstance *pInstance;

    uint32 PoisonBolt_Timer;
    uint32 Despawn_Timer;

    void EnterEvadeMode()
    {
        PoisonBolt_Timer = 5000+rand()%5000;
        Despawn_Timer = 30000;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }

    void JustDied(Unit *killer)
    {
        if(pInstance)
        {
            Creature *Vashj = NULL;
            Vashj = (Creature*)(Unit::GetUnit((*m_creature), pInstance->GetData64("LadyVashj")));

            if(Vashj)
                ((boss_lady_vashjAI*)Vashj->AI())->EventTaintedElementalDeath();
        }
    }

    void AttackStart(Unit *who)
    {
        if(!who && who != m_creature)
        return;

        if (who->isTargetableForAttack() && who!= m_creature)
        {
            m_creature->AddThreat(who, 0.1f);
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || m_creature->getVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessablePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {
            float attackRadius = m_creature->GetAttackDistance(who);
            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
            {
                if(who->HasStealthAura())
                who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

                m_creature->AddThreat(who, 0.1f);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //PoisonBolt_Timer
        if(PoisonBolt_Timer < diff)
        {
            Unit *target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM, 0);

            if(target && target->IsWithinDistInMap(m_creature, 30))
                DoCast(target, SPELL_POISON_BOLT);

            PoisonBolt_Timer = 5000+rand()%5000;
        }else PoisonBolt_Timer -= diff;

        //Despawn_Timer
        if(Despawn_Timer < diff)
        {
            //call Unsummon()
            m_creature->setDeathState(DEAD);

            //to prevent crashes
            Despawn_Timer = 1000;
        }else Despawn_Timer -= diff;
    }
};

//Fathom Sporebat
//Toxic Spores: Used in Phase 3 by the Spore Bats, it creates a contaminated green patch of ground, dealing about 2775-3225 nature damage every second to anyone who stands in it.
struct MANGOS_DLL_DECL mob_fathom_sporebatAI : public ScriptedAI
{
    mob_fathom_sporebatAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData()) ? ((ScriptedInstance*)c->GetInstanceData()) : NULL;
        EnterEvadeMode();
    }

    ScriptedInstance *pInstance;
     
    uint32 ToxicSpore_Timer;
    uint32 Check_Timer;
 
    void EnterEvadeMode()
    {
        m_creature->setFaction(14);
        ToxicSpore_Timer = 5000;
        Check_Timer = 1000;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
    }

    void AttackStart(Unit *who)
    {
        if(!who && who != m_creature)
            return;

        if (who->isTargetableForAttack() && who!= m_creature)
        {
            DoStartMeleeAttack(who);
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || m_creature->getVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessablePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {
            float attackRadius = m_creature->GetAttackDistance(who);
            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
            {
                if(who->HasStealthAura())
                who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

                DoStartMeleeAttack(who);
            }
        }
    }
 
    void UpdateAI (const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;
     
        //ToxicSpore_Timer
        if(ToxicSpore_Timer < diff)
        {
            Unit *target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM, 0);

            //The Spores will hit you anywhere in the instance: underwater, at the elevator, at the entrance, wherever.
            if(target)
                DoCast(target, SPELL_TOXIC_SPORES);

            ToxicSpore_Timer = 20000+rand()%5000;
        }else ToxicSpore_Timer -= diff;

        //Check_Timer
        if(Check_Timer < diff)
        {
            if(pInstance)
            {    
                //check if vashj is death
                Unit *Vashj = NULL;
                Vashj = Unit::GetUnit((*m_creature), pInstance->GetData64("LadyVashj"));
                if(!Vashj || (Vashj && !Vashj->isAlive()))
                {
                    //remove
                    m_creature->setDeathState(DEAD);
                    m_creature->RemoveCorpse();
                    m_creature->setFaction(35);
                }
            }

            Check_Timer = 1000;
        }else Check_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

//Coilfang Elite
//It's an elite Naga mob with 170,000 HP. It does about 5000 damage on plate, and has a nasty cleave hitting for about 7500 damage
CreatureAI* GetAI_mob_coilfang_elite(Creature *_Creature)
{
    SimpleAI* ai = new SimpleAI (_Creature);

    ai->m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);

    ai->Spell[0].Enabled = true;
    ai->Spell[0].Spell_Id = 31345;           //Cleave
    ai->Spell[0].Cooldown = 15000;
    ai->Spell[0].CooldownRandomAddition = 5000;
    ai->Spell[0].First_Cast = 5000;
    ai->Spell[0].Cast_Target_Type = CAST_HOSTILE_RANDOM;

    ai->EnterEvadeMode();

    return ai;
}

//Coilfang Strifer
//It hits plate for about 8000 damage, has a Mind Blast spell doing about 3000 shadow damage, and a Psychic Scream Aura, which fears everybody in a 8 yard range of it every 2-3 seconds , for 5 seconds and increasing their movement speed by 150% during the fear.
CreatureAI* GetAI_mob_coilfang_strider(Creature *_Creature)
{
    SimpleAI* ai = new SimpleAI (_Creature);

    ai->Spell[0].Enabled = true;
    ai->Spell[0].Spell_Id = 41374;           //Mind Blast
    ai->Spell[0].Cooldown = 30000;
    ai->Spell[0].CooldownRandomAddition = 10000;
    ai->Spell[0].First_Cast = 8000;
    ai->Spell[0].Cast_Target_Type = CAST_HOSTILE_TARGET;

    //Scream aura not implemented

    ai->EnterEvadeMode();

    return ai;
}

struct MANGOS_DLL_DECL mob_shield_generator_channelAI : public ScriptedAI
{
    mob_shield_generator_channelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData()) ? ((ScriptedInstance*)c->GetInstanceData()) : NULL;
        EnterEvadeMode();
    }

    ScriptedInstance *pInstance;

    uint32 Check_Timer;
    bool Channeled;
 
    void EnterEvadeMode()
    {
        Check_Timer = 1000;
        Channeled = false;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
        
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID , 11686);  //invisible
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void AttackStart(Unit *who) { return; }

    void MoveInLineOfSight(Unit *who) { return; }
 
    void UpdateAI (const uint32 diff)
    {
        if(!pInstance)
            return;

        if(!Channeled)
        {
            Unit *Vashj = NULL;
            Vashj = Unit::GetUnit((*m_creature), pInstance->GetData64("LadyVashj"));
            
            if(Vashj && Vashj->isAlive())
            {
                //start visual channel
                m_creature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, Vashj->GetGUID());
                m_creature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_MAGIC_BARRIER);
                Channeled = true;
            }
        }
    }
};

bool ItemUse_item_tainted_core(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    ScriptedInstance *pInstance = (player->GetInstanceData()) ? ((ScriptedInstance*)player->GetInstanceData()) : NULL;
    
    if(!pInstance)
    {
        player->GetSession()->SendNotification("Instance script not initialized");
        return true;
    }

    Creature *Vashj = NULL;
    Vashj = (Creature*)(Unit::GetUnit((*player), pInstance->GetData64("LadyVashj")));
    if(Vashj && ((boss_lady_vashjAI*)Vashj->AI())->Phase == 2)
    {
        if(targets.getGOTarget() && targets.getGOTarget()->GetTypeId()==TYPEID_GAMEOBJECT)
        {
            char *identifier;
            uint8 channel_identifier;
            switch(targets.getGOTarget()->GetEntry())
            {
            case 185052:
                identifier = "ShieldGenerator1";
                channel_identifier = 0;
                break;

            case 185053:
                identifier = "ShieldGenerator2";
                channel_identifier = 1;
                break;

            case 185051:
                identifier = "ShieldGenerator3";
                channel_identifier = 2;
                break;

            case 185054:
                identifier = "ShieldGenerator4";
                channel_identifier = 3;
                break;

            default:
                return true;
                break;
            }

            if(pInstance->GetData(identifier))
            {
                player->GetSession()->SendNotification("Already deactivated");
                return true;
            }

            //get and remove channel
            Unit *Channel = NULL;
            Channel = Unit::GetUnit((*Vashj), ((boss_lady_vashjAI*)Vashj->AI())->ShieldGeneratorChannel[channel_identifier]);
            if(Channel)
            {
                //call Unsummon()
                Channel->setDeathState(JUST_DIED);
            }

            pInstance->SetData(identifier, 1);

            //remove this item
            player->RemoveItemCount(31088, 1, true);
        }
    }
    return true;
}

CreatureAI* GetAI_boss_lady_vashj(Creature *_Creature)
{
    return new boss_lady_vashjAI (_Creature);
}
CreatureAI* GetAI_mob_enchanted_elemental(Creature *_Creature)
{
    return new mob_enchanted_elementalAI (_Creature);
}
CreatureAI* GetAI_mob_tainted_elemental(Creature *_Creature)
{
    return new mob_tainted_elementalAI (_Creature);
}
CreatureAI* GetAI_mob_fathom_sporebat(Creature *_Creature)
{
    return new mob_fathom_sporebatAI (_Creature);
}
CreatureAI* GetAI_mob_shield_generator_channel(Creature *_Creature)
{
    return new mob_shield_generator_channelAI (_Creature);
}

void AddSC_boss_lady_vashj()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_lady_vashj";
    newscript->GetAI = GetAI_boss_lady_vashj;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_enchanted_elemental";
    newscript->GetAI = GetAI_mob_enchanted_elemental;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_tainted_elemental";
    newscript->GetAI = GetAI_mob_tainted_elemental;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_fathom_sporebat";
    newscript->GetAI = GetAI_mob_fathom_sporebat;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_coilfang_elite";
    newscript->GetAI = GetAI_mob_coilfang_elite;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_coilfang_strider";
    newscript->GetAI = GetAI_mob_coilfang_strider;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="mob_shield_generator_channel";
    newscript->GetAI = GetAI_mob_shield_generator_channel;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="item_tainted_core";
    newscript->pItemUse = ItemUse_item_tainted_core;
    m_scripts[nrscripts++] = newscript;
}
