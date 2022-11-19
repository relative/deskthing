import { SpTopic, storeMap } from './store'
import { DTWS } from './ws'

export const dtws = new DTWS()

export function publishAllStores() {
  for (const topic in storeMap) {
    const store = storeMap[topic as SpTopic]
    dtws.publish(topic, store.getState())
  }
}
