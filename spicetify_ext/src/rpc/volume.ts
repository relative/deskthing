import { registerRpcEvent } from './handlers'

registerRpcEvent(
  'com.spotify.superbird.volume.volume_down',
  async ({ reply }) => {
    await Spicetify.Platform.PlaybackAPI.lowerVolume()
    reply()
  }
)

registerRpcEvent(
  'com.spotify.superbird.volume.volume_up',
  async ({ reply }) => {
    await Spicetify.Platform.PlaybackAPI.raiseVolume()
    reply()
  }
)
