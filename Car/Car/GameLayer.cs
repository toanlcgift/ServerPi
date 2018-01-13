using System.Diagnostics;
using System.Collections.Generic;
using CocosSharp;
using Microsoft.Xna.Framework;
using Sockets.Plugin;
using System;

namespace Car
{
    public class GameLayer : CCLayerColor
    {

        // Define a label variable
        CCSprite spriteUp1;
        CCSprite spriteUp2;
        CCSprite spriteUp3;
        CCSprite spriteUp4;
        CCSprite spriteDown1;
        CCSprite spriteDown2;
        CCSprite spriteDown3;
        CCSprite spriteDown4;
        TcpSocketClient client;

        public GameLayer() : base(CCColor4B.Yellow)
        {

            spriteUp1 = new CCSprite("Up.png");
            spriteUp2 = new CCSprite("Up.png");
            spriteUp3 = new CCSprite("Up.png");
            spriteUp4 = new CCSprite("Up.png");

            spriteDown1 = new CCSprite("Up.png") { Rotation = 180 };
            spriteDown2 = new CCSprite("Up.png") { Rotation = 180 };
            spriteDown3 = new CCSprite("Up.png") { Rotation = 180 };
            spriteDown4 = new CCSprite("Up.png") { Rotation = 180 };

            var touchListener = new CCEventListenerTouchAllAtOnce();
            touchListener.OnTouchesBegan = TouchBegan;
            touchListener.OnTouchesEnded = TouchEnd;
            AddEventListener(touchListener);
            AddChild(spriteUp1);
            AddChild(spriteUp2);
            AddChild(spriteUp3);
            AddChild(spriteUp4);
        }

        private async void TouchEnd(List<CCTouch> arg1, CCEvent arg2)
        {
            client.WriteStream.WriteByte(Convert.ToByte('4'));
            await client.WriteStream.FlushAsync();
        }

        private async void TouchBegan(List<CCTouch> arg1, CCEvent arg2)
        {
            if (arg1.Count > 0)
            {
                //if (spriteUp1.BoundingBox.ContainsPoint(arg1[0].Location))
                //{
                //    client.WriteStream.WriteByte(Convert.ToByte('2'));
                //    await client.WriteStream.FlushAsync();
                //}
                //else if (spriteLeft.BoundingBox.ContainsPoint(arg1[0].Location))
                //{
                //    client.WriteStream.WriteByte(Convert.ToByte('1'));
                //    await client.WriteStream.FlushAsync();
                //}
                //else if (spriteRight.BoundingBox.ContainsPoint(arg1[0].Location))
                //{
                //    client.WriteStream.WriteByte(Convert.ToByte('3'));
                //    await client.WriteStream.FlushAsync();
                //}
            }
            //await client.DisconnectAsync();
        }

        protected override async void AddedToScene()
        {
            base.AddedToScene();

            // Use the bounds to layout the positioning of our drawable assets
            var bounds = VisibleBoundsWorldspace;


            spriteUp1.PositionX = bounds.Center.X / 4;
            spriteUp1.PositionY = 4 * bounds.Center.Y / 3;
            // position the label on the center of the screen
            //spriteUp.Position = bounds.Center;

            client = new TcpSocketClient();
            try
            {
                await client.ConnectAsync("192.168.17.103", 51717);
            }
            catch { }
        }
    }
}

