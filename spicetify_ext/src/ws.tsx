import { processRpc, RpcMessage } from './rpc'
import { SpTopic, storeMap } from './store'
import { publishAllStores } from './util'
import console from './console'

const { setTimeout, clearTimeout } = window

const DEFAULT_RECONNECT_DELAY = 1_000

export class DTWS {
  url: URL
  ws?: WebSocket

  reconnectDelay = DEFAULT_RECONNECT_DELAY
  maxDelay = 32_000
  timerReconnect?: number

  constructor() {
    this.url = new URL('ws://localhost:36308')
    this.retry()
  }

  send(j: any) {
    if (this.ws?.readyState !== WebSocket.OPEN) return
    this.ws?.send(JSON.stringify(j))
  }

  onOpen = () => {
    this.reconnectDelay = DEFAULT_RECONNECT_DELAY
    publishAllStores()
  }
  onClose = (e: CloseEvent) => {
    const { maxDelay } = this
    if (this.reconnectDelay < maxDelay) this.reconnectDelay *= 2
    console.log('Reconnecting in', this.reconnectDelay, 'milliseconds')
    if (typeof this.timerReconnect !== 'undefined') {
      clearTimeout(this.timerReconnect)
      delete this.timerReconnect
    }
    this.timerReconnect = setTimeout(() => this.retry(), this.reconnectDelay)
  }

  onMessage = (e: MessageEvent) => {
    try {
      const m = JSON.parse(e.data.toString())
      if (m.topic) {
        const store = storeMap[m.topic as SpTopic]
        if (!store) throw 'Requested store does not exist'
        this.publish(m.topic, store.getState())
      } else if (m.proc) {
        processRpc(m as RpcMessage)
      }
    } catch (err) {
      console.error('msg e', err)
    }
  }
  onError = (e: Event) => {
    console.error('WebSocket error', e)
  }

  retry() {
    if (this.ws) {
      this.ws.close()
      delete this.ws
    }
    this.ws = new WebSocket(this.url)
    this.ws.addEventListener('open', this.onOpen)
    this.ws.addEventListener('close', this.onClose)
    this.ws.addEventListener('error', this.onError)
    this.ws.addEventListener('message', this.onMessage)
  }

  publish(topic: string, state: any) {
    this.send({ topic, state })
  }
}
