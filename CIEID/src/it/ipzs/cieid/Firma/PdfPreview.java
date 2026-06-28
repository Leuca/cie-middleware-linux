package it.ipzs.cieid.Firma;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Insets;
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.Border;

import org.ghost4j.document.DocumentException;
import org.ghost4j.document.PDFDocument;
import org.ghost4j.renderer.RendererException;
import org.ghost4j.renderer.SimpleRenderer;

public class PdfPreview {
    private JPanel prPanel;
    private String filePath;
    private String signImagePath;
    private int pdfPageIndex;
    private int pdfNumPages;
	private List<Image> images;			    
	private JLabel imgLabel;
    private ImageIcon imgIcon;
    private MoveablePicture signImage;
    private JPanel imgPanel;
    // Size of the page bitmap as actually drawn inside imgPanel. The page is
    // letterboxed (centered) within the panel, so these differ from the panel
    // size and must be used as the reference when converting the signature box
    // position into page fractions.
    private int imgWidth;
    private int imgHeight;
    
    public PdfPreview(JPanel panelPdfPreview, String pdfFilePath, String signImagePath)
    {
    	this.prPanel = panelPdfPreview;
    	this.filePath = pdfFilePath;
    	this.signImagePath = signImagePath;
    	this.pdfPageIndex = 0;
    	imgIcon = new ImageIcon();
    	imgLabel = new JLabel();
    	imgPanel = new JPanel();
		imgPanel.setLayout(new BorderLayout(0,0));
		imgPanel.setBackground(Color.white);
		signImage = new MoveablePicture(signImagePath);
		imgPanel.add(signImage);
		imgPanel.add(imgLabel);
		
		try {
			PDFDocument document = new PDFDocument();
			document.load(new File(filePath));		
			pdfNumPages = document.getPageCount();
			System.out.println("Pdf page: " + pdfNumPages);
		    SimpleRenderer renderer = new SimpleRenderer();
		    
		    renderer.setResolution(100);
		    prPanel.removeAll();
			images = renderer.render(document);
			
			showPreview();

		    
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("PDF File not found");
			e.printStackTrace();
		} catch (DocumentException e) {
			// TODO Auto-generated catch block
			System.out.println("Document Exception");
			e.printStackTrace();
		} catch (RendererException e) {
			// TODO Auto-generated catch block
			System.out.println("Renderer Exception");
			e.printStackTrace();
		}
    }
    
    public void showPreview()
    {
    	Image tmpImg = images.get(pdfPageIndex);

    	int width = prPanel.getWidth();
    	int height = prPanel.getHeight();
    	
    	int tmpImgWidth = tmpImg.getWidth(null);
    	int tmpImgHeight =  tmpImg.getHeight(null);
    	
	// Remember the previous page size so the signature box can be re-anchored
	// to the same relative spot when the preview is re-rendered at a new size.
	int prevImgWidth = imgWidth;
	int prevImgHeight = imgHeight;

	imgHeight = height;
	imgWidth = width;
    
    	double signImgMult = 1.0;

    	if( tmpImgWidth > tmpImgHeight)
    	{
    		imgHeight  = (int)(width*tmpImgHeight)/tmpImgWidth;
    		
    		if(imgHeight > height)
    		{
    			imgWidth = (int)(height*tmpImgWidth)/tmpImgHeight;
				imgHeight = (int)(imgWidth*tmpImgHeight)/tmpImgWidth;
    		}
    	}else
    	{
    		imgWidth = (int)(height*tmpImgWidth)/tmpImgHeight;
                    
            if(imgWidth > width)
            {
            	imgHeight = (int)(width*tmpImgHeight)/tmpImgWidth;
        		imgWidth = (int)(imgHeight*tmpImgWidth)/tmpImgHeight;
            }
    	}
    	
		signImgMult = imgWidth / 300.0;
    	
		signImage.setSize((int)(50.0 * signImgMult), (int)(25.0 * signImgMult));
		// Re-anchor the box to the same relative page position. Without this, a
		// box placed while the preview was larger keeps its old pixel coordinates;
		// divided by the now-smaller page they yield fractions > 1 (off-page).
		if (prevImgWidth > 0 && prevImgHeight > 0)
		{
			int nx = (int)((long) signImage.getX() * imgWidth / prevImgWidth);
			int ny = (int)((long) signImage.getY() * imgHeight / prevImgHeight);
			nx = Math.max(0, Math.min(nx, imgWidth - signImage.getWidth()));
			ny = Math.max(0, Math.min(ny, imgHeight - signImage.getHeight()));
			signImage.setLocation(nx, ny);
		}
		signImage.reloadImage();
		imgIcon.setImage(tmpImg.getScaledInstance(imgWidth, imgHeight, Image.SCALE_AREA_AVERAGING));
		imgLabel.setIcon(imgIcon);
		imgLabel.setHorizontalAlignment(JLabel.CENTER);
		imgLabel.setVerticalAlignment(JLabel.CENTER);
	    imgLabel.revalidate();
	    imgLabel.repaint();
		
		//imgPanel.removeAll();
		imgPanel.setMaximumSize(new Dimension(imgWidth, imgHeight));
		imgPanel.updateUI();
		
		prPanel.removeAll();
		prPanel.add(imgPanel);
		prPanel.updateUI();
    }
    
    public void prevImage()
    {
		if((pdfPageIndex -1) >= 0)
		{
			pdfPageIndex -= 1;
		}
		
		showPreview();
    }
    
    public void nextImage()
    {
		if((pdfPageIndex + 1) < pdfNumPages)
		{
			pdfPageIndex += 1;
		}
		
		showPreview();
    }
    
    public int getSelectedPage()
    {
    	return pdfPageIndex;
    }
    
    public float[] signImageInfos()
    {
    	float infos[] = new float[4];
    	
	// The page image exactly fills imgPanel (imgPanel == imgLabel == icon), so the
	// signature box position in panel coordinates maps directly to page fractions.
	// showPreview() keeps the box anchored to the page across resizes.
	float x = ((float)signImage.getX() / (float)imgWidth);
	float y = ((float)(signImage.getY() + signImage.getHeight()) / (float)imgHeight);
	float w = ((float)signImage.getWidth() / (float)imgWidth);
	float h = ((float)signImage.getHeight() / (float)imgHeight);
    	
    	infos[0] = x;
    	infos[1] = y;
    	infos[2] = w;
    	infos[3] = h;
    	
    	return infos;
    }
    
}
