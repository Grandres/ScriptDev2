/* Copyright (C) 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../../sc_defines.h"

#define SAY_AGGRO1 "Glory to the master!"
#define SAY_AGGRO2 "Your life is forfeit!"
#define SAY_AGGRO3 "Die, trespasser!"
#define SAY_SUMMON "Rise, my soldiers! Rise and fight once more!"
#define SAY_SLAY1 "My task is done!"
#define SAY_SLAY2 "Breathe no more!"
#define SAY_DEATH "I will serve the master... in... death!"
#define SOUND_AGGRO1      8845
#define SOUND_AGGRO2      8846
#define SOUND_AGGRO3      8847
#define SOUND_SUMMON      8851
#define SOUND_SLAY1      8849
#define SOUND_SLAY2      8849
#define SOUND_DEATH      8848

// Teleport position of Noth on his balcony
#define TELE_X 2631.370
#define TELE_Y -3529.680
#define TELE_Z 274.040
#define TELE_O 6.277

#define SPELL_BLINK                           29211      
#define SPELL_CRIPPLE                         29212
#define SPELL_CURSEPLAGUEBRINGER              28213
#define SPELL_WRATHPLAGUEBRINGER              28214

// IMPORTANT: BALCONY TELEPORT NOT ADDED YET! WILL BE ADDED SOON!

struct MANGOS_DLL_DECL boss_nothAI : public ScriptedAI
{
    boss_nothAI(Creature *c) : ScriptedAI(c) {EnterEvadeMode();}

    uint32 Blink_Timer;
    uint32 Curse_Timer;
    uint32 Wrath_Timer;
    uint32 Summon_Timer;
    bool InCombat;

    void EnterEvadeMode()
    {       
        Blink_Timer = 25000;
        Curse_Timer = 4000;
	Wrath_Timer = 9000;
	Summon_Timer = 12000;
        InCombat = false;

        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop();
        DoGoHome();
        m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISARM, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPTED, true);       
	m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZED, true);
	m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
	m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SILENCE, true);
    }


    void AttackStart(Unit *who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack() && who!= m_creature)
        {
            DoStartMeleeAttack(who);
                //Say our dialog on initial aggro
                if (!InCombat)
                {
                    switch (rand()%3)
                    {
                    case 0:
                           DoYell(SAY_AGGRO1,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO1);
                        break;
                    case 1:
                           DoYell(SAY_AGGRO2,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO2);
                        break;
                    case 2:
                           DoYell(SAY_AGGRO3,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO3);
                        break;
                    }
                    InCombat = true;
                }
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
                if (!InCombat)
                {
                    switch (rand()%3)
                    {
                    case 0:
                           DoYell(SAY_AGGRO1,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO1);
                        break;
                    case 1:
                           DoYell(SAY_AGGRO2,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO2);
                        break;
                    case 2:
                           DoYell(SAY_AGGRO3,LANG_UNIVERSAL,NULL);
                           DoPlaySoundToSet(m_creature,SOUND_AGGRO3);
                        break;
                    }
                    InCombat = true;
                }

            }
        }
    }

    void KilledUnit(Unit* victim)
    {

                    switch (rand()%2)
                    {
                    case 0:
                        DoYell(SAY_SLAY1,LANG_UNIVERSAL,NULL);
			DoPlaySoundToSet(m_creature,SOUND_SLAY1);
                        break;
                    case 1:
                        DoYell(SAY_SLAY2,LANG_UNIVERSAL,NULL);
			DoPlaySoundToSet(m_creature,SOUND_SLAY2);
                        break;

                    }
    }


    void JustDied(Unit* Killer)
    {
        DoYell(SAY_DEATH,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,SOUND_DEATH);
    }


    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostilTarget())
            return;

        //Check if we have a current target
        if( m_creature->getVictim() && m_creature->isAlive())
        {

            //Blink_Timer
            if (Blink_Timer < diff)
            {
                //Cast
                DoCast(m_creature->getVictim(),SPELL_CRIPPLE);
                DoCast(m_creature,SPELL_BLINK);

                //25 seconds
                Blink_Timer = 25000;
            }else Blink_Timer -= diff;

            //Curse_Timer
            if (Curse_Timer < diff)
            {

                //Cast
                DoCast(m_creature->getVictim(),SPELL_CURSEPLAGUEBRINGER);

                //28 seconds until we should cast this agian
                Curse_Timer = 28000;
            }else Curse_Timer -= diff;

            //Wrath_Timer
            if (Wrath_Timer < diff)
            {
                //Cast
                DoCast(m_creature->getVictim(),SPELL_WRATHPLAGUEBRINGER);

                //18 seconds until we should cast this agian
                Wrath_Timer = 18000;
            }else Wrath_Timer -= diff;

            //Summon_Timer
            if (Summon_Timer < diff)
            {

                    DoYell(SAY_SUMMON,LANG_UNIVERSAL,NULL);
                    DoPlaySoundToSet(m_creature,SOUND_SUMMON);
                    Unit* target = NULL;
                    Unit* SummonedSkeletons = NULL;

                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);
                    SummonedSkeletons = m_creature->SummonCreature(16984,2684.804,-3502.517,261.313,0,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,80000);

                    if (SummonedSkeletons)
                    {
                        target = SelectUnit(SELECT_TARGET_RANDOM,0);
                        if (target)
                            SummonedSkeletons->AddThreat(target,1.0f);
                    }

                //30 seconds until we should cast this agian
                Summon_Timer = 30500;
            } else Summon_Timer -= diff;
            


            //If we are within range melee the target
            if( m_creature->IsWithinDistInMap(m_creature->getVictim(), ATTACK_DISTANCE))
            {
                //Make sure our attack is ready and we arn't currently casting
                if( m_creature->isAttackReady() && !m_creature->m_currentSpell)
                {
                    m_creature->AttackerStateUpdate(m_creature->getVictim());
                    m_creature->resetAttackTimer();
                }
            }
        }
    }
}; 
CreatureAI* GetAI_boss_noth(Creature *_Creature)
{
    return new boss_nothAI (_Creature);
}


void AddSC_boss_noth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_noth";
    newscript->GetAI = GetAI_boss_noth;
    m_scripts[nrscripts++] = newscript;
}


