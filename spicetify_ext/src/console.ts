const origConsole = { ...console }

export default {
  log: origConsole.log.bind(null, '(deskthing-bridge)'),
  info: origConsole.info.bind(null, '(deskthing-bridge)'),
  warn: origConsole.warn.bind(null, '(deskthing-bridge)'),
  error: origConsole.error.bind(null, '(deskthing-bridge)'),
}
