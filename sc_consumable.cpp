// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simcraft.h"

// ==========================================================================
// Consumable
// ==========================================================================

// ==========================================================================
// Flask
// ==========================================================================

struct flask_t : public action_t
{
  int8_t type;

  flask_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "flask", p ), type( FLASK_NONE )
  {
    std::string type_str;

    option_t options[] =
    {
      { "type", OPT_STRING, &type_str },
      { NULL }
    };
    parse_options( options, options_str );

    trigger_gcd = 0;
    harmful = false;
    for( int i=0; i < FLASK_MAX; i++ )
    {
      if( type_str == util_t::flask_type_string( i ) )
      {
	type = i;
	break;
      }
    }
    assert( type != FLASK_NONE );
  }
  
  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s uses Flask %s", player -> name(), util_t::flask_type_string( type ) );
    player -> flask = type;
    switch( type )
    {
    case FLASK_BLINDING_LIGHT:     
      player -> spell_power[ SCHOOL_ARCANE ] += 80;
      player -> spell_power[ SCHOOL_HOLY   ] += 80;
      player -> spell_power[ SCHOOL_NATURE ] += 80;
      break;
    case FLASK_DISTILLED_WISDOM:
      player -> attribute[ ATTR_INTELLECT ] += 65;
      break;
    case FLASK_ENDLESS_RAGE:
      player -> attack_power += 180;
      break;
    case FLASK_FROST_WYRM:
      player -> spell_power[ SCHOOL_MAX ] += 125;
      break;
    case FLASK_MIGHTY_RESTORATION:
      player -> mp5 += 25;
      break;
    case FLASK_PURE_DEATH:
      player -> spell_power[ SCHOOL_FIRE   ] += 80;
      player -> spell_power[ SCHOOL_FROST  ] += 80;
      player -> spell_power[ SCHOOL_SHADOW ] += 80;
      break;
    case FLASK_RELENTLESS_ASSAULT:
      player -> attack_power += 120;
      break;
    case FLASK_SUPREME_POWER:
      player -> spell_power[ SCHOOL_MAX ] += 70;
      break;
    default: assert(0);
    }
  }

  virtual bool ready()
  {
    return( player -> flask           ==  FLASK_NONE &&
	    player -> elixir_guardian == ELIXIR_NONE &&
	    player -> elixir_battle   == ELIXIR_NONE );
  }
};

// ==========================================================================
// Flask
// ==========================================================================

struct food_t : public action_t
{
  int8_t type;

  food_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "food", p ), type( FOOD_NONE )
  {
    std::string type_str;

    option_t options[] =
    {
      { "type", OPT_STRING, &type_str },
      { NULL }
    };
    parse_options( options, options_str );

    trigger_gcd = 0;
    harmful = false;
    for( int i=0; i < FOOD_MAX; i++ )
    {
      if( type_str == util_t::food_type_string( i ) )
      {
	type = i;
	break;
      }
    }
    assert( type != FOOD_NONE );
  }
  
  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s uses Food %s", player -> name(), util_t::food_type_string( type ) );
    player -> food = type;
    switch( type )
    {
    case FOOD_TENDER_SHOVELTUSK_STEAK:     
      player -> spell_power[ SCHOOL_MAX ] += 46;
      player -> attribute[ ATTR_STAMINA ] += 40;
      break;
    case FOOD_SNAPPER_EXTREME:     
      player -> spell_hit += 40;
      player -> attribute[ ATTR_STAMINA ] += 40;
      break;
    case FOOD_SMOKED_SALMON:
      player -> spell_power[ SCHOOL_MAX ] += 35;
      player -> attribute[ ATTR_STAMINA ] += 40;
      break;
    case FOOD_POACHED_BLUEFISH:     
      player -> spell_power[ SCHOOL_MAX ] += 23;
      player -> attribute[ ATTR_SPIRIT ]  += 20;
      break;
    case FOOD_BLACKENED_BASILISK:     
      player -> spell_power[ SCHOOL_MAX ] += 23;
      player -> attribute[ ATTR_SPIRIT ]  += 20;
      break;
    case FOOD_GOLDEN_FISHSTICKS:     
      player -> spell_power[ SCHOOL_MAX ] += 23;
      player -> attribute[ ATTR_SPIRIT ]  += 20;
      break;
    case FOOD_CRUNCHY_SERPENT:     
      player -> spell_power[ SCHOOL_MAX ] += 23;
      player -> attribute[ ATTR_SPIRIT ]  += 20;
      break;
    case FOOD_GREAT_FEAST:     
      player -> attack_power              += 60;
      player -> spell_power[ SCHOOL_MAX ] += 35;
	  player -> attribute[ ATTR_STAMINA ] += 30;
      break;
    default: assert(0);
    }
  }

  virtual bool ready()
  {
    return( player -> food        ==  FOOD_NONE );
  }
};

// ==========================================================================
// Destruction Potion
// ==========================================================================

struct destruction_potion_t : public action_t
{
  int8_t used;

  destruction_potion_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "destruction_potion", p ), used( 0 )
  {
    cooldown = 120.0;
    cooldown_group = "potion";
    trigger_gcd = 0;
    harmful = false;
  }
  
  virtual void execute()
  {
    struct expiration_t : public event_t
    {
      expiration_t( sim_t* sim, player_t* p ) : event_t( sim, p )
      {
	name = "Destruction Potion Expiration";
	player -> aura_gain( "Destruction Potion Buff" );
	player -> spell_power[ SCHOOL_MAX ] += 120;
	player -> spell_crit += 0.02;
	sim -> add_event( this, 15.0 );
      }
      virtual void execute()
      {
	player -> aura_loss( "Destruction Potion Buff" );
	player -> spell_power[ SCHOOL_MAX ] -= 120;
	player -> spell_crit -= 0.02;
      }
    };
  
    player -> share_cooldown( cooldown_group, cooldown );
    new ( sim ) expiration_t( sim, player );
    used = sim -> potion_sickness;
  }

  virtual bool ready()
  {
    if( used )
      return false;

    return( cooldown_ready > sim -> current_time );
  }

  virtual void reset()
  {
    action_t::reset();
    used = 0;
  }
};

// ==========================================================================
// Mana Potion
// ==========================================================================

struct mana_potion_t : public action_t
{
  int16_t trigger;
  int16_t min;
  int16_t max;
  int8_t  used;

  mana_potion_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "mana_potion", p ), trigger(0), min(0), max(0), used(0)
  {
    option_t options[] =
    {
      { "min",     OPT_INT16, &min     },
      { "max",     OPT_INT16, &max     },
      { "trigger", OPT_INT16, &trigger },
      { NULL }
    };
    parse_options( options, options_str );

    if( min == 0 && max == 0) min = max = trigger;

    if( min > max) { int16_t tmp = min; min = max; max = tmp;}    

    if( max == 0 ) max = trigger;
    if( trigger == 0 ) trigger = max;
    assert( max > 0 && trigger > 0 );

    cooldown = 120.0;
    cooldown_group = "potion";
    trigger_gcd = 0;
    harmful = false;
  }
  
  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s uses Mana potion", player -> name() );
    int16_t delta = max - min;
    // FIXME! I hope the gain between min and max are distributed uniformly
    double gain   = min + delta * player -> sim -> rng -> real();
    player -> resource_gain( RESOURCE_MANA, gain, player -> gains.mana_potion );
    player -> share_cooldown( cooldown_group, cooldown );
    used = sim -> potion_sickness;
  }

  virtual bool ready()
  {
    if( used )
      return false;

    if( cooldown_ready > sim -> current_time ) 
      return false;

    return( player -> resource_max    [ RESOURCE_MANA ] - 
	    player -> resource_current[ RESOURCE_MANA ] ) > trigger;
  }

  virtual void reset()
  {
    action_t::reset();
    used = 0;
  }
};

// ==========================================================================
// Health Stone
// ==========================================================================

struct health_stone_t : public action_t
{
  int16_t trigger;
  int16_t health;
  int8_t  used;

  health_stone_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "health_stone", p ), trigger(0), health(0), used(0)
  {
    option_t options[] =
    {
      { "health",  OPT_INT16, &health  },
      { "trigger", OPT_INT16, &trigger },
      { NULL }
    };
    parse_options( options, options_str );

    if( health  == 0 ) health = trigger;
    if( trigger == 0 ) trigger = health;
    assert( health > 0 && trigger > 0 );

    cooldown = 120.0;
    cooldown_group = "rune";
    trigger_gcd = 0;
    harmful = false;
  }
  
  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s uses Health Stone", player -> name() );
    player -> resource_gain( RESOURCE_HEALTH, health );
    player -> share_cooldown( cooldown_group, cooldown );
    used = sim -> potion_sickness;
  }

  virtual bool ready()
  {
    if( used )
      return false;

    if( cooldown_ready > sim -> current_time ) 
      return false;

    return( player -> resource_max    [ RESOURCE_HEALTH ] - 
	    player -> resource_current[ RESOURCE_HEALTH ] ) > trigger;
  }

  virtual void reset()
  {
    action_t::reset();
    used = 0;
  }
};

// ==========================================================================
// Dark Rune
// ==========================================================================

struct dark_rune_t : public action_t
{
  int16_t trigger;
  int16_t health;
  int16_t mana;
  int8_t  used;

  dark_rune_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "dark_rune", p ), trigger(0), health(0), mana(0), used(0)
  {
    option_t options[] =
    {
      { "trigger", OPT_INT16,  &trigger },
      { "mana",    OPT_INT16,  &mana    },
      { "health",  OPT_INT16,  &health  },
      { NULL }
    };
    parse_options( options, options_str );

    if( mana    == 0 ) mana = trigger;
    if( trigger == 0 ) trigger = mana;
    assert( mana > 0 && trigger > 0 );

    cooldown = 120.0;
    cooldown_group = "rune";
    trigger_gcd = 0;
    harmful = false;
  }
  
  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s uses Dark Rune", player -> name() );
    player -> resource_gain( RESOURCE_MANA,   mana, player -> gains.dark_rune );
    player -> resource_loss( RESOURCE_HEALTH, health );
    player -> share_cooldown( cooldown_group, cooldown );
    used = sim -> potion_sickness;
  }

  virtual bool ready()
  {
    if( used )
      return false;

    if( cooldown_ready > sim -> current_time ) 
      return false;

    if( player -> resource_current[ RESOURCE_HEALTH ] <= health )
      return false;

    return( player -> resource_max    [ RESOURCE_MANA ] - 
	    player -> resource_current[ RESOURCE_MANA ] ) > trigger;
  }

  virtual void reset()
  {
    action_t::reset();
    used = 0;
  }
};

// Wizard Oil Action =========================================================

struct wizard_oil_t : public action_t
{
  int16_t bonus_power;
  int16_t bonus_crit;

  wizard_oil_t( player_t* p, const std::string& options_str ) : 
    action_t( ACTION_USE, "wizard_oil", p )
  {
    trigger_gcd = 0;

    bonus_power = (int16_t) util_t::ability_rank( p -> level,  56,72,  42,68,  36,0 );

    bonus_crit = ( p -> level <= 55 ? 14 : 0 );
  }

  virtual void execute()
  {
    if( sim -> log ) report_t::log( sim, "%s performs %s", player -> name(), name() );

    player -> main_hand_weapon.buff = WIZARD_OIL;
    player -> spell_power[ SCHOOL_MAX ] += bonus_power;
    player -> spell_crit                += bonus_crit / player -> rating.spell_crit;
  };

  virtual bool ready()
  {
    return( player -> main_hand_weapon.buff != WIZARD_OIL );
  }
};

// ==========================================================================
// consumable_t::init_flask
// ==========================================================================

void consumable_t::init_flask( player_t* p )
{
  // Eventually, flask will be taken off the actions= directive.
}

// ==========================================================================
// consumable_t::init_elixirs
// ==========================================================================

void consumable_t::init_elixirs( player_t* p )
{
  // Eventually, elixirs will be taken off the actions= directive.
}

// ==========================================================================
// consumable_t::init_food
// ==========================================================================

void consumable_t::init_food( player_t* p )
{
  // Eventually, food will be taken off the actions= directive.
}

// ==========================================================================
// consumable_t::create_action
// ==========================================================================

action_t* consumable_t::create_action( player_t*          p,
				       const std::string& name, 
				       const std::string& options_str )
{
  if( name == "dark_rune"          ) return new          dark_rune_t( p, options_str );
  if( name == "destruction_potion" ) return new destruction_potion_t( p, options_str );
  if( name == "flask"              ) return new              flask_t( p, options_str );
  if( name == "food"               ) return new               food_t( p, options_str );
  if( name == "health_stone"       ) return new       health_stone_t( p, options_str );
  if( name == "mana_potion"        ) return new        mana_potion_t( p, options_str );
  if( name == "wizard_oil"         ) return new         wizard_oil_t( p, options_str );

  return 0;
}
