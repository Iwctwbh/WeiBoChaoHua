爬取微博超话
xmake + qt

# 教程
以弹丸论破为例，http://weibo.com/p/1008088be585959d6c54bfd57d41717a50c9ea

只需要将 /p/ 之后的 id(1008088be585959d6c54bfd57d41717a50c9ea) 替换 config.json 文件中 Url 字段的 containerid= 即可

## 实例
### 弹丸论破 http://weibo.com/p/1008088be585959d6c54bfd57d41717a50c9ea

config.json 中应为
```
{
  "Url": "https://m.weibo.cn/api/container/getIndex?containerid=1008088be585959d6c54bfd57d41717a50c9ea",
  "Cookie": "",
  "Count": 10000
}
```

### 全国 https://m.weibo.cn/p/index?containerid=100808b6bd56e6ef22833885effa4ad18876c2

config.json 中应为
```
{
  "Url": "https://m.weibo.cn/api/container/getIndex?containerid=100808b6bd56e6ef22833885effa4ad18876c2",
  "Cookie": "",
  "Count": 10000
}
```
