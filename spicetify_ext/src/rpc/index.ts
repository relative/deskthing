import { handlers } from './handlers'

export interface RpcMessage {
  id: number
  proc: string
  args: any[]
  argsKw: object
}

export function processRpc(m: RpcMessage) {
  const { id, proc, args, argsKw } = m

  console.log('Received RPC call', id, proc, args, argsKw)

  if (handlers[proc]) {
    for (const handler of handlers[proc]) {
      try {
        handler(m)
      } catch (err) {}
    }
  }
}
