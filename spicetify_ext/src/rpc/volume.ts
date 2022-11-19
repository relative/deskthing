import { registerRpcEvent } from './handlers'

const VOLUME_STEPS = 15
const oneStep = 1 / VOLUME_STEPS

registerRpcEvent('com.spotify.superbird.volume.volume_up', ({}) => {
  Spicetify.Player.setVolume(
    Math.min(Spicetify.Player.getVolume() + oneStep, 1)
  )
})
registerRpcEvent('com.spotify.superbird.volume.volume_down', ({}) => {
  Spicetify.Player.setVolume(
    Math.max(0, Spicetify.Player.getVolume() - oneStep)
  )
})
