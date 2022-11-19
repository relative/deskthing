import { SpTopic, storeMap, volumeStore } from './store'
import { dtws } from './util'
import console from './console'

// import rpc handler files here so that the registerRpcEvent call doesnt fail at runtime
import './rpc/volume'

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
}

async function main() {
  while (!Spicetify?.showNotification) {
    await new Promise(resolve => setTimeout(resolve, 100))
  }

  setupListeners()
}

export default main
