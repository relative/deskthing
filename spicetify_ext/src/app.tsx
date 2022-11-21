import {
  contextStore,
  DTSPlayQueueItem,
  DTSRefPair,
  DTSTrack,
  playerStore,
  playQueueStore,
  SpTopic,
  storeMap,
  volumeStore,
} from './store'
import { dtws } from './util'
import console from './console'

// import rpc handler files here so that the registerRpcEvent call doesnt fail at runtime
import './rpc/volume'
import './rpc/player'
import './rpc/image'
import './rpc/remoteconfig'
import './rpc/gql'

export const changeHandler =
  <T,>(topic: string) =>
  (state: T, prevState: T) => {
    dtws.publish(topic, state)
  }

// wrap "fn" in a try/catch handler
const _ =
  (fn: Function) =>
  (...args: any[]) => {
    try {
      return fn.apply(null, args)
    } catch (e) {
      //if (e instanceof AssertionError) return
      console.error('Error calling', fn.name, e)
    }
  }

interface PlaybackRestrictions {
  canRepeatContext: boolean
  canRepeatTrack: boolean
  canSeek: boolean
  canSkipPrev: boolean
  canSkipNext: boolean
  canToggleShuffle: boolean
}

function setupListeners() {
  for (const topic in storeMap) {
    const store = storeMap[topic as SpTopic]
    store.subscribe(changeHandler<ReturnType<typeof store['getState']>>(topic))
  }

  Spicetify.Platform.PlaybackAPI.getEvents().addListener(
    'volume',
    _(({ data: { volume } }: any) => {
      volumeStore.setState({ volume })
    })
  )

  interface UnifiedPlayerState {
    context: {
      uri: string
    }
    restrictions: PlaybackRestrictions

    repeat: {
      value: number
      track: boolean
      context: boolean
    }
    shuffle: boolean

    paused: boolean
    buffering: boolean

    playback: {
      speed: number
      position: number // ms
      duration: number // ms
    }

    track: DTSTrack
  }

  const convertArtist = (d: any): DTSRefPair => {
    return {
      name: d.name,
      uri: d.uri,
    }
  }
  const getImageId = (d: any): string | null => {
    return Array.isArray(d.images) && d.images.length > 0
      ? d.images[0].url || null
      : null
  }
  const convertTrack = (d: any): DTSTrack => {
    const isSaved: boolean =
      d.metadata && d.metadata['collection.in_collection'] === 'true'

    const mediaType = d.mediaType
    const t: DTSTrack = {
      album: {
        name: d.album?.name,
        uri: d.album?.uri,
      },
      artist: convertArtist(d.artists[0]),
      artists: d.artists.map((a: any) => convertArtist(a)),
      duration_ms: d.duration.milliseconds,
      image_bytes: null,
      image_id: getImageId(d.images),
      name: d.name,
      saved: isSaved,
      uid: d.uid,
      uri: d.uri,
      is_episode: false,
      is_podcast: false,
    }
    return t
  }

  const convertQueueItem = (d: any): DTSPlayQueueItem => {
    const { contextTrack, provider } = d
    const { metadata } = contextTrack

    let artists: DTSPlayQueueItem['artists'] = []
    for (const key in metadata) {
      let name, uri
      if (key === 'artist_name') {
        name = metadata['artist_name']
        uri = metadata['artist_uri']
      } else if (key.startsWith('artist_name:')) {
        let id = key.substring('artist_name:'.length)
        name = metadata['artist_name:' + id]
        uri = metadata['artist_uri:' + id]
      }
      if (name && uri) {
        artists.push({
          a: name,
          b: uri,
          name,
          uri,
        })
      }
    }
    let t = {
      artist_name: metadata['artist_name'],
      artists,
      image_uri: metadata['image_url'],
      name: metadata['title'],
      provider: provider,
      uid: contextTrack.uid,
      uri: contextTrack.uri,
    }
    return t
  }

  Spicetify.Platform.PlayerAPI.getEvents().addListener(
    'update',
    _(({ data }: any) => {
      const u: UnifiedPlayerState = {
        context: {
          uri: data.context?.uri,
        },
        restrictions: {
          canRepeatContext: data.restrictions?.canToggleRepeatContext,
          canRepeatTrack: data.restrictions?.canToggleRepeatTrack,
          canSeek: data.restrictions?.canSeek,
          canSkipPrev: data.restrictions?.canSkipPrevious,
          canSkipNext: data.restrictions?.canSkipNext,
          canToggleShuffle: data.restrictions?.canToggleShuffle,
        },
        repeat: {
          value: data.repeat,
          track: data.repeat === 1,
          context: data.repeat === 2,
        },
        shuffle: data.shuffle,
        paused: data.isPaused,
        buffering: data.isBuffering,

        playback: {
          speed: data.speed,
          position: data.positionAsOfTimestamp,
          duration: data.item?.duration?.milliseconds,
        },

        track: convertTrack(data.item),
      }

      contextStore.setState({
        can_repeat_context: u.restrictions.canRepeatContext,
        can_repeat_track: u.restrictions.canRepeatTrack,
        can_shuffle: u.restrictions.canToggleShuffle,
        id: u.context.uri,
        repeat_context: u.repeat.context,
        repeat_track: u.repeat.track,
        shuffle: u.shuffle,
        subtitle: 'idk',
        title: 'idk either',
        type: 'your_library',
        uri: u.context.uri,
      })

      playerStore.setState({
        context_title: null,
        context_uri: u.context.uri,
        playback_options: {
          repeat: u.repeat.value,
          shuffle: u.shuffle,
        },
        playback_position: u.playback.position,
        playback_restrictions: {
          can_repeat_context: u.restrictions.canRepeatContext,
          can_repeat_track: u.restrictions.canRepeatTrack,
          can_seek: u.restrictions.canSeek,
          can_skip_next: u.restrictions.canSkipNext,
          can_skip_prev: u.restrictions.canSkipPrev,
          can_toggle_shuffle: u.restrictions.canToggleShuffle,
        },
        playback_speed: u.playback.speed,
        track: u.track,
        is_paused: u.paused,
        is_paused_bool: u.paused,
      })

      if (Spicetify.Queue) {
        playQueueStore.setState({
          current: convertQueueItem(Spicetify.Queue.track),
          next: Spicetify.Queue.nextTracks.map(d => convertQueueItem(d)),
          previous: Spicetify.Queue.prevTracks.map(d => convertQueueItem(d)),
        })
      }
    })
  )

  // Spicetify.Platform.PlayerAPI._queue.getEvents().addListener(
  //   'update',
  //   _(({ data }: any) => {})
  // )
}

async function main() {
  while (!Spicetify?.showNotification) {
    await new Promise(resolve => setTimeout(resolve, 100))
  }

  setupListeners()
}

export default main
