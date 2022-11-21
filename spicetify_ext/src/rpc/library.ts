import { registerRpcEvent } from './handlers'

registerRpcEvent<{ id: string }>(
  'com.spotify.get_saved',
  async ({ argsKw: { id }, reply }) => {
    // TODO
    const uri = id
    reply({
      can_save: true,
      saved: true,
      uri: uri,
    })
  }
)

registerRpcEvent<{
  limit: number
  offset: number
  parent_id: string
}>(
  'com.spotify.get_children_of_item',
  async ({ argsKw: { limit, offset, parent_id }, reply }) => {
    const uri = Spicetify.URI.from(parent_id)
    if (!uri) return
    console.log('Requested children for', uri)
  }
)
