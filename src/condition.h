#pragma once
#ifndef CATA_SRC_CONDITION_H
#define CATA_SRC_CONDITION_H

#include <functional>
#include <iosfwd>
#include <string>
#include <unordered_set>

#include "dialogue.h"
#include "mission.h"

class JsonObject;
namespace dialogue_data
{
// when updating this, please also update `dynamic_line_string_keys` in
// `lang/extract_json_string.py` so the lines are properly extracted for translation
const std::unordered_set<std::string> simple_string_conds = { {
        "u_male", "u_female", "npc_male", "npc_female",
        "has_no_assigned_mission", "has_assigned_mission", "has_many_assigned_missions",
        "has_no_available_mission", "has_available_mission", "has_many_available_missions",
        "mission_complete", "mission_incomplete", "mission_has_generic_rewards",
        "npc_available", "npc_following", "npc_friend", "npc_hostile",
        "npc_train_skills", "npc_train_styles", "npc_train_spells",
        "at_safe_space", "is_day", "npc_has_activity", "is_outside", "u_is_outside", "npc_is_outside", "u_has_camp",
        "u_can_stow_weapon", "npc_can_stow_weapon", "u_has_weapon", "npc_has_weapon",
        "u_driving", "npc_driving",
        "has_pickup_list", "is_by_radio", "has_reason"
    }
};
const std::unordered_set<std::string> complex_conds = { {
        "u_has_any_trait", "npc_has_any_trait", "u_has_trait", "npc_has_trait",
        "u_has_flag", "npc_has_flag", "npc_has_class", "u_has_mission", "u_monsters_in_direction", "u_safe_mode_trigger",
        "u_has_strength", "npc_has_strength", "u_has_dexterity", "npc_has_dexterity",
        "u_has_intelligence", "npc_has_intelligence", "u_has_perception", "npc_has_perception",
        "u_is_wearing", "npc_is_wearing", "u_has_item", "npc_has_item", "u_has_move_mode", "npc_has_move_mode",
        "u_has_items", "npc_has_items", "u_has_item_category", "npc_has_item_category",
        "u_has_bionics", "npc_has_bionics", "u_has_effect", "npc_has_effect", "u_need", "npc_need",
        "u_at_om_location", "u_near_om_location", "npc_at_om_location", "npc_near_om_location",
        "npc_role_nearby", "npc_allies", "npc_allies_global", "npc_service",
        "u_has_cash", "u_are_owed", "u_query", "npc_query", "u_has_item_with_flag", "npc_has_item_with_flag",
        "npc_aim_rule", "npc_engagement_rule", "npc_rule", "npc_override", "u_has_hp", "npc_has_hp",
        "npc_cbm_reserve_rule", "npc_cbm_recharge_rule", "u_has_faction_trust",
        "days_since_cataclysm", "is_season", "mission_goal", "u_has_var", "npc_has_var",
        "u_has_skill", "npc_has_skill", "u_know_recipe", "u_compare_var", "npc_compare_var",
        "u_compare_time_since_var", "npc_compare_time_since_var", "is_weather", "one_in_chance", "x_in_y_chance",
        "is_temperature", "is_windpower", "is_humidity", "is_pressure", "u_is_height", "npc_is_height",
        "u_has_worn_with_flag", "npc_has_worn_with_flag", "u_has_wielded_with_flag", "npc_has_wielded_with_flag",
        "u_has_pain", "npc_has_pain", "u_has_power", "npc_has_power", "u_has_focus", "npc_has_focus", "u_has_morale",
        "npc_has_morale", "u_is_on_terrain", "npc_is_on_terrain", "u_is_in_field", "npc_is_in_field", "compare_int", "compare_string"
    }
};
} // namespace dialogue_data

template<class T>
static dialogue copy_dialogue( const T &d )
{
    Creature *creature_alpha = d.has_alpha ? d.actor( false )->get_creature() : nullptr;
    item_location *item_alpha = d.has_alpha ? d.actor( false )->get_item() : nullptr;
    Creature *creature_beta = d.has_beta ? d.actor( true )->get_creature() : nullptr;
    item_location *item_beta = d.has_beta ? d.actor( true )->get_item() : nullptr;
    return dialogue(
               creature_alpha ? get_talker_for( creature_alpha ) : item_alpha ? get_talker_for(
                   item_alpha ) : nullptr,
               creature_beta ? get_talker_for( creature_beta ) : item_beta ? get_talker_for(
                   item_beta ) : nullptr
           );
}

template<class T>
struct str_or_var {
    cata::optional<std::string> str_val;
    cata::optional<var_info> var_val;
    cata::optional<std::string> default_val;
    std::string evaluate( const T &d ) const {
        if( str_val.has_value() ) {
            return str_val.value();
        } else if( var_val.has_value() ) {
            std::string val = read_var_value( var_val.value(), d );
            if( !val.empty() ) {
                return std::string( val );
            }
            return default_val.value();
        } else {
            debugmsg( "No valid value." );
            return "";
        }
    }
};

template<class T>
struct int_or_var_part {
    cata::optional<int> int_val;
    cata::optional<var_info> var_val;
    cata::optional<int> default_val;
    cata::optional<talk_effect_fun_t<T>> arithmetic_val;
    int evaluate( const T &d ) const {
        if( int_val.has_value() ) {
            return int_val.value();
        } else if( var_val.has_value() ) {
            std::string val = read_var_value( var_val.value(), d );
            if( !val.empty() ) {
                return std::stoi( val );
            }
            return default_val.value();
        } else if( arithmetic_val.has_value() ) {
            arithmetic_val.value()( d );
            var_info info = var_info( var_type::global, "temp_var" );
            std::string val = read_var_value( info, d );
            if( !val.empty() ) {
                return std::stoi( val );
            } else {
                debugmsg( "No valid value." );
                return 0;
            }
        } else {
            debugmsg( "No valid value." );
            return 0;
        }
    }
};

template<class T>
struct int_or_var {
    bool pair = false;
    int_or_var_part<T> min;
    int_or_var_part<T> max;
    int evaluate( const T &d ) const {
        if( pair ) {
            return rng( min.evaluate( d ), max.evaluate( d ) );
        } else {
            return min.evaluate( d );
        }
    }
};

template<class T>
struct duration_or_var_part {
    cata::optional<time_duration> dur_val;
    cata::optional<var_info> var_val;
    cata::optional<time_duration> default_val;
    cata::optional<talk_effect_fun_t<T>> arithmetic_val;
    time_duration evaluate( const T &d ) const {
        if( dur_val.has_value() ) {
            return dur_val.value();
        } else if( var_val.has_value() ) {
            std::string val = read_var_value( var_val.value(), d );
            if( !val.empty() ) {
                time_duration ret_val;
                ret_val = time_duration::from_turns( std::stoi( val ) );
                return ret_val;
            }
            return default_val.value();
        } else if( arithmetic_val.has_value() ) {
            arithmetic_val.value()( d );
            var_info info = var_info( var_type::global, "temp_var" );
            std::string val = read_var_value( info, d );
            if( !val.empty() ) {
                time_duration ret_val;
                ret_val = time_duration::from_turns( std::stoi( val ) );
                return ret_val;
            } else {
                debugmsg( "No valid value." );
                return 0_seconds;
            }
        } else {
            debugmsg( "No valid value." );
            return 0_seconds;
        }
    }
};

template<class T>
struct duration_or_var {
    bool pair = false;
    duration_or_var_part<dialogue> min;
    duration_or_var_part<dialogue> max;
    time_duration evaluate( const T &d ) const {
        if( pair ) {
            return rng( min.evaluate( d ), max.evaluate( d ) );
        } else {
            return min.evaluate( d );
        }
    }
};
template<class T>
str_or_var<T> get_str_or_var( const JsonValue &jv, const std::string &member, bool required = true,
                              const std::string &default_val = "" );
template<class T>
int_or_var<T> get_int_or_var( const JsonObject &jo, std::string member, bool required = true,
                              int default_val = 0 );
template<class T>
int_or_var_part<T> get_int_or_var_part( const JsonValue &jv, const std::string &member,
                                        bool required = true,
                                        int default_val = 0 );
template<class T>
duration_or_var<T> get_duration_or_var( const JsonObject &jo, std::string member,
                                        bool required = true,
                                        time_duration default_val = 0_seconds );
template<class T>
duration_or_var_part<T> get_duration_or_var_part( const JsonValue &jv, const std::string &member,
        bool required = true,
        time_duration default_val = 0_seconds );
template<class T>
tripoint_abs_ms get_tripoint_from_var( cata::optional<var_info> var, const T &d );
var_info read_var_info( const JsonObject &jo, bool require_default );
void write_var_value( var_type type, const std::string &name, talker *talk,
                      const std::string &value );
template<class T>
std::string get_talk_varname( const JsonObject &jo, const std::string &member,
                              bool check_value, int_or_var<T> &default_val );
std::string get_talk_var_basename( const JsonObject &jo, const std::string &member,
                                   bool check_value );
// the truly awful declaration for the conditional_t loading helper_function
template<class T>
void read_condition( const JsonObject &jo, const std::string &member_name,
                     std::function<bool( const T & )> &condition, bool default_val );

/**
 * A condition for a response spoken by the player.
 * This struct only adds the constructors which will load the data from json
 * into a lambda, stored in the std::function object.
 * Invoking the function operator with a dialog reference (so the function can access the NPC)
 * returns whether the response is allowed.
 */
template<class T>
struct conditional_t {
    private:
        std::function<bool( const T & )> condition;

    public:
        conditional_t() = default;
        explicit conditional_t( const std::string &type );
        explicit conditional_t( const JsonObject &jo );

        void set_has_any_trait( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_trait( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_flag( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_var( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_compare_var( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_compare_time_since_var( const JsonObject &jo, const std::string &member,
                                         bool is_npc = false );
        void set_has_activity( bool is_npc = false );
        void set_is_riding( bool is_npc = false );
        void set_npc_has_class( const JsonObject &jo, bool is_npc );
        void set_u_has_mission( const JsonObject &jo );
        void set_u_monsters_in_direction( const JsonObject &jo );
        void set_u_safe_mode_trigger( const JsonObject &jo );
        void set_has_strength( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_dexterity( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_intelligence( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_perception( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_hp( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_is_deaf( bool is_npc = false );
        void set_is_on_terrain( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_is_in_field( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_one_in_chance( const JsonObject &jo, const std::string &member );
        void set_query( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_x_in_y_chance( const JsonObject &jo, const std::string &member );
        void set_has_worn_with_flag( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_wielded_with_flag( const JsonObject &jo, const std::string &member,
                                        bool is_npc = false );
        void set_is_wearing( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_item( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_items( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_item_with_flag( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_item_category( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_bionics( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_effect( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_need( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_at_om_location( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_near_om_location( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_has_move_mode( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_npc_role_nearby( const JsonObject &jo );
        void set_npc_allies( const JsonObject &jo );
        void set_npc_allies_global( const JsonObject &jo );
        void set_u_has_cash( const JsonObject &jo );
        void set_u_are_owed( const JsonObject &jo );
        void set_npc_aim_rule( const JsonObject &jo, bool is_npc );
        void set_npc_engagement_rule( const JsonObject &jo, bool is_npc );
        void set_npc_cbm_reserve_rule( const JsonObject &jo, bool is_npc );
        void set_npc_cbm_recharge_rule( const JsonObject &jo, bool is_npc );
        void set_npc_rule( const JsonObject &jo, bool is_npc );
        void set_npc_override( const JsonObject &jo, bool is_npc );
        void set_days_since( const JsonObject &jo );
        void set_is_season( const JsonObject &jo );
        void set_is_weather( const JsonObject &jo );
        void set_mission_goal( const JsonObject &jo, bool is_npc );
        void set_has_faction_trust( const JsonObject &jo, const std::string &member );
        void set_no_assigned_mission();
        void set_has_assigned_mission();
        void set_has_many_assigned_missions();
        void set_no_available_mission( bool is_npc );
        void set_has_available_mission( bool is_npc );
        void set_has_many_available_missions( bool is_npc );
        void set_mission_complete( bool is_npc );
        void set_mission_incomplete( bool is_npc );
        void set_npc_available( bool is_npc );
        void set_npc_following( bool is_npc );
        void set_npc_friend( bool is_npc );
        void set_npc_hostile( bool is_npc );
        void set_npc_train_skills( bool is_npc );
        void set_npc_train_styles( bool is_npc );
        void set_npc_train_spells( bool is_npc );
        void set_at_safe_space( bool is_npc );
        void set_can_stow_weapon( bool is_npc = false );
        void set_has_weapon( bool is_npc = false );
        void set_is_driving( bool is_npc = false );
        void set_is_day();
        void set_has_stolen_item( bool is_npc = false );
        void set_is_outside( bool is_npc = false );
        void set_is_underwater( bool is_npc = false );
        void set_is_by_radio();
        void set_u_has_camp();
        void set_has_pickup_list( bool is_npc );
        void set_has_reason();
        void set_is_gender( bool is_male, bool is_npc = false );
        void set_has_skill( const JsonObject &jo, const std::string &member, bool is_npc = false );
        void set_u_know_recipe( const JsonObject &jo, const std::string &member );
        void set_mission_has_generic_rewards();
        void set_can_see( bool is_npc = false );
        void set_compare_string( const JsonObject &jo, const std::string &member );
        void set_compare_int( const JsonObject &jo, const std::string &member );
        static std::function<int( const T & )> get_get_int( const JsonObject &jo );
        static std::function<int( const T & )> get_get_int( const std::string &value,
                const JsonObject &jo );
        bool operator()( const T &d ) const {
            if( !condition ) {
                return false;
            }
            return condition( d );
        }
};

#if !defined(MACOSX)
extern template struct conditional_t<dialogue>;
extern template void read_condition<dialogue>( const JsonObject &jo, const std::string &member_name,
        std::function<bool( const dialogue & )> &condition, bool default_val );
extern template struct conditional_t<mission_goal_condition_context>;
extern template void read_condition<mission_goal_condition_context>( const JsonObject &jo,
        const std::string &member_name,
        std::function<bool( const mission_goal_condition_context & )> &condition, bool default_val );
#endif

#endif // CATA_SRC_CONDITION_H
