using System.Diagnostics;
using System.Collections.Generic;
using CocosSharp;
using Microsoft.Xna.Framework;
using Sockets.Plugin;
using System;
using System.Linq;

namespace Car
{
    public class GameLayer : CCLayerColor
    {

        // Define a label variable
        CCSprite spriteUp1;
        CCSprite spriteUp2;
        CCSprite spriteDown1;
        CCSprite spriteDown2;
        TcpSocketClient client;

        public GameLayer() : base(CCColor4B.Transparent)
        {

            spriteUp1 = new CCSprite("Up.png");
            spriteUp2 = new CCSprite("Up.png");

            spriteDown1 = new CCSprite("Up.png") { Rotation = 180 };
            spriteDown2 = new CCSprite("Up.png") { Rotation = 180 };

            var touchListener = new CCEventListenerTouchAllAtOnce();
            touchListener.OnTouchesBegan = TouchBegan;
            touchListener.OnTouchesEnded = TouchEnd;
            AddEventListener(touchListener);
            AddChild(spriteUp1);
            AddChild(spriteUp2);
            AddChild(spriteDown1);
            AddChild(spriteDown2);
        }

        private async void TouchEnd(List<CCTouch> arg1, CCEvent arg2)
        {
            if (arg1.Count > 0)
            {
                try
                {
                    if (arg1.Any(x => spriteDown1.BoundingBox.ContainsPoint(x.Location)))
                    {
                        client.WriteStream.WriteByte(Convert.ToByte('e'));
                        await client.WriteStream.FlushAsync();
                    }
                    else if (arg1.Any(x => spriteDown2.BoundingBox.ContainsPoint(x.Location)))
                    {
                        client.WriteStream.WriteByte(Convert.ToByte('h'));
                        await client.WriteStream.FlushAsync();
                    }
                    else if (arg1.Any(x => spriteUp1.BoundingBox.ContainsPoint(x.Location)))
                    {
                        client.WriteStream.WriteByte(Convert.ToByte('e'));
                        await client.WriteStream.FlushAsync();
                    }
                    else if (arg1.Any(x => spriteUp2.BoundingBox.ContainsPoint(x.Location)))
                    {
                        client.WriteStream.WriteByte(Convert.ToByte('h'));
                        await client.WriteStream.FlushAsync();
                    }
                }
                catch
                {

                }
            }
            else
            {
                try
                {
                    client.WriteStream.WriteByte(Convert.ToByte('i'));
                    await client.WriteStream.FlushAsync();
                }
                catch { }
            }
        }

        private async void TouchBegan(List<CCTouch> arg1, CCEvent arg2)
        {
            if (arg1.Count > 0)
            {
                if (arg1.Any(x => spriteUp1.BoundingBox.ContainsPoint(x.Location)))
                {
                    client.WriteStream.WriteByte(Convert.ToByte('c'));
                    await client.WriteStream.FlushAsync();
                }
                else if (arg1.Any(x => spriteUp2.BoundingBox.ContainsPoint(x.Location)))
                {
                    client.WriteStream.WriteByte(Convert.ToByte('f'));
                    await client.WriteStream.FlushAsync();
                }
                else if (arg1.Any(x => spriteDown1.BoundingBox.ContainsPoint(x.Location)))
                {
                    client.WriteStream.WriteByte(Convert.ToByte('d'));
                    await client.WriteStream.FlushAsync();
                }
                else if (arg1.Any(x => spriteDown2.BoundingBox.ContainsPoint(x.Location)))
                {
                    client.WriteStream.WriteByte(Convert.ToByte('g'));
                    await client.WriteStream.FlushAsync();
                }

            }
            //await client.DisconnectAsync();
        }

        protected override async void AddedToScene()
        {
            base.AddedToScene();

            // Use the bounds to layout the positioning of our drawable assets
            var bounds = VisibleBoundsWorldspace;


            spriteUp1.PositionX = bounds.Center.X - spriteUp1.ContentSize.Width;
            spriteUp1.PositionY = 4 * bounds.Center.Y / 3;

            spriteUp2.PositionX = bounds.Center.X + spriteUp2.ContentSize.Width;
            spriteUp2.PositionY = 4 * bounds.Center.Y / 3;

            spriteDown1.PositionX = bounds.Center.X - spriteDown1.ContentSize.Width;
            spriteDown1.PositionY = 2 * bounds.Center.Y / 3;

            spriteDown2.PositionX = bounds.Center.X + spriteDown2.ContentSize.Width;
            spriteDown2.PositionY = 2 * bounds.Center.Y / 3;

            client = new TcpSocketClient();
            try
            {
                await client.ConnectAsync(App.IPAddress, 51717);
            }
            catch { }
        }
    }
}

