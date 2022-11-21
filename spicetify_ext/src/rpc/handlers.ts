import { RpcMessage } from './'

export type HandlerFn<T = any> = (msg: RpcMessage<T>) => any | Promise<any>
export const handlers: Record<string, HandlerFn[]> = {}

export function registerRpcEvent<T = any>(
  _procs: string | string[],
  handler: HandlerFn<T>
) {
  let procs = []
  if (!Array.isArray(_procs)) {
    procs = [_procs]
  } else procs = _procs
  for (const proc of procs) {
    handlers[proc] = handlers[proc] || []
    handlers[proc].push(handler)
  }
}
