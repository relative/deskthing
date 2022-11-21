import { registerRpcEvent } from './handlers'
import console from '../console'

registerRpcEvent<{
  client_info: { serial: string; version_os: string; version_software: string }
}>(
  'com.spotify.superbird.remote_configuration',
  async ({ argsKw: { client_info }, reply }) => {
    console.log('Remote config', client_info)
    reply({
      result: {
        use_volume_superbird_namespace: true,
        use_playerstate_superbird_namespace: true,
        use_superbird_namespace: true,
        graphql_endpoint_enabled: true,

        non_spotify_playback_android: true,
        non_spotify_playback_ios: false,

        night_mode_enabled: true,
        night_mode_slope: 10,
        night_mode_strength: 40,

        developer_menu_enabled: true, // false

        upload_wakeword: false,

        tips_enabled: true,
        tips_on_demand_enabled: true,
        tips_startup_delay: 600,
        tips_request_interval: 900,
        tips_show_time: 20,
        tips_track_change_delay: 10,
        tips_interaction_delay: 4,

        handle_incoming_phone_calls: false,

        long_press_settings_power_off_v2: true,

        get_home_enabled: true,
        hide_home_more_button: true,

        volume_control: true,
        use_relative_volume_control: true,

        queue_enabled: true,
        use_new_voice_ui: true,

        podcast_trailer_enabled: true,
        tracklist_context_menu_enabled: true,
        ota_inactivity_timeout: 10,
        podcast_speed_change_enabled: true,
        auto_restart_after_ota: false,

        app_launch_rssi_limit: -1000,
        error_messaging_no_network: true,
        local_command_stop_enabled: true,

        enable_push_to_talk_shelf: false,
        enable_push_to_talk_npv: false,
        graphql_for_shelf_enabled: true,

        log_signal_strength: true,
        log_requests: true,
        batch_ubi_logs: true,
      },
    })
  }
)
