import create from 'zustand/vanilla'

interface DTSSession {
  is_offline: boolean
  is_in_forced_offline_mode: boolean
  is_logged_in: boolean
  connection_type: string
}
interface DTSStatus {
  long_text: string
  short_text: string
  code: number
}
interface DTSPodcast {
  podcast_playback_speed: number
}
interface DTSVolume {
  volume: number
  volume_steps: number
}
interface DTSContext {
  can_repeat_context: boolean
  can_repeat_track: boolean
  can_shuffle: boolean
  id: string
  repeat_context: boolean
  repeat_track: boolean
  shuffle: boolean
  subtitle: string
  title: string
  type: string
  uri: string
}
export interface DTSRefPair {
  name: string
  uri: string
}
export interface DTSTrack {
  album: DTSRefPair
  artist: DTSRefPair
  artists: DTSRefPair[]
  duration_ms: number
  image_bytes: null
  image_id: string | null
  name: string
  saved: boolean
  uid: string
  uri: string
  is_episode: boolean
  is_podcast: boolean
}

// why
interface DTSPlayQueueItemArtist {
  a: string // name
  b: string // uri
  name: string
  uri: string
}
export interface DTSPlayQueueItem {
  artist_name: string
  artists: DTSPlayQueueItemArtist[]
  image_uri: string
  name: string
  provider: string
  uid: string
  uri: string
}
interface DTSPlayQueue {
  current?: DTSPlayQueueItem
  next?: DTSPlayQueueItem[] // n items
  previous?: DTSPlayQueueItem[] // 10 items
}
interface DTSPlayer {
  currently_active_application: null
  context_title: null
  context_uri: string
  playback_options: {
    repeat: number
    shuffle: boolean
  }
  playback_position: number
  playback_restrictions: {
    can_repeat_context: boolean
    can_repeat_track: boolean
    can_seek: boolean
    can_skip_next: boolean
    can_skip_prev: boolean
    can_toggle_shuffle: boolean
  }
  playback_speed: number
  track: DTSTrack
  is_paused: boolean
  is_paused_bool: boolean
}
const contextStore = create<DTSContext>(() => ({
  can_repeat_context: true,
  can_repeat_track: true,
  can_shuffle: true,
  id: 'spotify:user:snip:collection',
  repeat_context: true,
  repeat_track: false,
  shuffle: true,
  subtitle: 'Playing from Your Library',
  title: 'Liked Songs',
  type: 'your_library',
  uri: 'spotify:user:snip:collection',
}))
const playQueueStore = create<DTSPlayQueue>(() => ({}))
const sessionStore = create<DTSSession>(() => ({
  is_offline: false,
  is_in_forced_offline_mode: false,
  is_logged_in: true,
  connection_type: 'wlan',
}))
const statusStore = create<DTSStatus>(() => ({
  long_text: '',
  short_text: '',
  code: 0,
}))
const podcastStore = create<DTSPodcast>(() => ({ podcast_playback_speed: 1 }))
const volumeStore = create<DTSVolume>(() => ({ volume: 1, volume_steps: 15 }))

const playerStore = create<DTSPlayer>(() => ({
  currently_active_application: null,
  context_title: null,
  context_uri: 'spotify:user:snip:collection',
  playback_options: {
    repeat: 2,
    shuffle: true,
  },
  playback_position: 353,
  playback_restrictions: {
    can_repeat_context: true,
    can_repeat_track: true,
    can_seek: true,
    can_skip_next: true,
    can_skip_prev: true,
    can_toggle_shuffle: true,
  },
  playback_speed: 1,
  track: {
    album: {
      name: 'snip',
      uri: 'spotify:album:snip',
    },
    artist: {
      name: 'snip',
      uri: 'spotify:artist:snip',
    },
    artists: [
      {
        name: 'snip',
        uri: 'spotify:artist:snip',
      },
      {
        name: 'snip',
        uri: 'spotify:artist:snip',
      },
      {
        name: 'snip',
        uri: 'spotify:artist:snip',
      },
    ],
    duration_ms: 196800,
    image_bytes: null,
    image_id: 'spotify:image:snip',
    name: 'snip',
    saved: true,
    uid: 'snip',
    uri: 'spotify:track:snip',
    is_episode: false,
    is_podcast: false,
  },
  is_paused: false,
  is_paused_bool: false,
}))

export const storeMap = {
  'com.spotify.current_context': contextStore,
  'com.spotify.play_queue': playQueueStore,
  'com.spotify.session_state': sessionStore,
  'com.spotify.status': statusStore,
  'com.spotify.podcast_playback_speed': podcastStore,
  'com.spotify.superbird.volume.volume_state': volumeStore,

  'com.spotify.superbird.player_state': playerStore,
}

export type SpTopic = keyof typeof storeMap

export {
  contextStore,
  playQueueStore,
  sessionStore,
  statusStore,
  podcastStore,
  volumeStore,
  playerStore,
}
