import { registerRpcEvent } from './handlers'

registerRpcEvent<{
  payload: string
}>('com.spotify.superbird.graphql', async ({ argsKw: { payload }, reply }) => {
  const bearerToken = `Bearer ${Spicetify.Platform.Session.accessToken}`

  const res = await fetch(
    'https://spclient.wg.spotify.com/superbird-graph/v1/query',
    {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        Authorization: bearerToken,
      },
      body: JSON.stringify({ query: payload }),
    }
  )

  const json = await res.json()
  reply({ data: json.data, errors: json.errors })
})
