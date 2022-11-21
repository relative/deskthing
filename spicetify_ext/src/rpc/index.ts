import { handlers } from './handlers'
import console from '../console'
import { dtws } from '../util'

export interface RpcMessage<T = object> {
  devId: number
  reqId: number
  proc: string
  args: any[]
  argsKw: T

  replied: boolean
  reply: (args?: any, details?: any, argsKw?: any) => void
}

export function processRpc(m: RpcMessage) {
  m.replied = false
  m.reply = (args = [], details = {}, argsKw = {}) => {
    if (m.replied) throw new Error('This message has already been replied to')
    dtws.send({
      result: {
        devId: m.devId,
        reqId: m.reqId,
        details,
        args,
        argsKw,
      },
    })
    m.replied = true
  }
  const { reqId, proc, args, argsKw } = m

  if (handlers[proc]) {
    for (const handler of handlers[proc]) {
      try {
        handler(m)
      } catch (err) {}
    }
  } else {
    console.log('Received unhandled RPC call', proc, argsKw)
  }
}
