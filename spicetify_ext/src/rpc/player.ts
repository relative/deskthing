import { registerRpcEvent } from './handlers'

registerRpcEvent<{
  playback_speed: number
}>('com.spotify.set_playback_speed', async ({ argsKw: { playback_speed } }) => {
  console.log('New playback speed:', playback_speed)
  if (playback_speed === 0) {
    await Spicetify.Platform.PlayerAPI.pause()
  } else if (playback_speed === 1) {
    await Spicetify.Platform.PlayerAPI.resume()
  } else {
    await Spicetify.Platform.PlayerAPI.setSpeed(playback_speed)
  }
})

registerRpcEvent(
  ['com.spotify.pause', 'com.spotify.superbird.pause'],
  async () => {
    await Spicetify.Platform.PlayerAPI.pause()
  }
)

registerRpcEvent(
  ['com.spotify.resume', 'com.spotify.superbird.resume'],
  async () => {
    await Spicetify.Platform.PlayerAPI.resume()
  }
)

registerRpcEvent(
  ['com.spotify.skip_previous', 'com.spotify.superbird.skip_previous'],
  async () => {
    await Spicetify.Platform.PlayerAPI.skipToPrevious()
  }
)
registerRpcEvent(
  ['com.spotify.skip_next', 'com.spotify.superbird.skip_next'],
  async () => {
    await Spicetify.Platform.PlayerAPI.skipToNext()
  }
)
