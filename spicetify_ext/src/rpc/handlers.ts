import { RpcMessage } from './'

export type HandlerFn = (msg: RpcMessage) => any | Promise<any>
export const handlers: Record<string, HandlerFn[]> = {}

export function registerRpcEvent(proc: string, handler: HandlerFn) {
  handlers[proc] = handlers[proc] || []
  handlers[proc].push(handler)
}
