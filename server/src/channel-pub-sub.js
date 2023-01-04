class PubSub {
  constructor() {
    // 缓存队列
    this._events = {}
  }

  // 注册
  subscribe(event, callback) {
    if (this._events[event]) {
      // 如果当前 event 存在，所以我们只需要往后添加当前次监听操作
      this._events[event].push(callback)
    } else {
      // 之前没有订阅过此事件
      this._events[event] = [callback]
    }
  }

  unsubscribe(event, callback) {
    if (this._events[event]) {
      const i = this._events[event].findIndex(v => v == callback)
      if (i != -1) this._events[event].splice(i, 1)
    }
  }

  // 发布
  publish(event, ...args) {
    const items = this._events[event]
    if (items && items.length) {
      items.forEach(function (callback) {
        callback.call(this, ...args)
      })
    }
  }
}

module.exports = new PubSub()
